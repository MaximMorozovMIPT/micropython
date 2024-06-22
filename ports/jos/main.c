#include "modio.h"
#include "lexer.h"

#include "py/lexer.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/objstr.h"
#include "py/stackctrl.h"
#include "py/mphal.h"
#include "py/mpthread.h"
#include "extmod/misc.h"
#include "extmod/modplatform.h"
#include "extmod/vfs.h"
#include "extmod/vfs_posix.h"
#include "genhdr/mpversion.h"
#include "input.h"

// Command line options, with their defaults
static bool compile_only = false;
static uint emit_opt = MP_EMIT_OPT_NONE;

#if MICROPY_ENABLE_GC
// Heap size of GC heap (if enabled)
// Make it larger on a 64 bit machine, because pointers are larger.
long heap_size = 1024 * 1024 * (sizeof(mp_uint_t) / 4);
#endif

// Number of heaps to assign by default if MICROPY_GC_SPLIT_HEAP=1
#ifndef MICROPY_GC_SPLIT_HEAP_N_HEAPS
#define MICROPY_GC_SPLIT_HEAP_N_HEAPS (1)
#endif

#if !MICROPY_PY_SYS_PATH
#error "The unix port requires MICROPY_PY_SYS_PATH=1"
#endif

#if !MICROPY_PY_SYS_ARGV
#error "The unix port requires MICROPY_PY_SYS_ARGV=1"
#endif

static void stderr_print_strn(void *env, const char *str, size_t len) {
    (void)env;
    cprintf("%.*s", (int)len, str);
    mp_os_dupterm_tx_strn(str, len);
}
const mp_print_t mp_stderr_print = {NULL, stderr_print_strn};

void mp_hal_set_interrupt_char(__attribute__((unused)) char c) {
}

#define FORCED_EXIT (0x100)
// If exc is SystemExit, return value where FORCED_EXIT bit set,
// and lower 8 bits are SystemExit value. For all other exceptions,
// return 1.
static int handle_uncaught_exception(mp_obj_base_t *exc) {
    // check for SystemExit
    if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(exc->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
        cprintf("[ERROR] SystemExit exception\n");
        // None is an exit value of 0; an int is its value; anything else is 1
        mp_obj_t exit_val = mp_obj_exception_get_value(MP_OBJ_FROM_PTR(exc));
        mp_int_t val = 0;
        if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
            val = 1;
        }
        return FORCED_EXIT | (val & 255);
    }

    // Report all other exceptions
    cprintf("[ERROR] Unhandled exception\n");
    mp_obj_print_exception(&mp_stderr_print, MP_OBJ_FROM_PTR(exc));
    return 1;
}

void __attribute__((noreturn)) nlr_jump_fail(void *val) {
    #if MICROPY_USE_READLINE == 1
    mp_hal_stdio_mode_orig();
    #endif
    cprintf("[ERROR] Uncaught NLR %p\n", val);
    exit(1);
}

#define LEX_SRC_STR (1)
#define LEX_SRC_VSTR (2)
#define LEX_SRC_FILENAME (3)
#define LEX_SRC_STDIN (4)

// Returns standard error codes: 0 for success, 1 for all other errors,
// except if FORCED_EXIT bit is set then script raised SystemExit and the
// value of the exit is in the lower 8 bits of the return value
static int execute_from_lexer(int source_kind, const void *source, mp_parse_input_kind_t input_kind, bool is_repl) {
    mp_hal_set_interrupt_char(CHAR_CTRL_C);

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // create lexer based on source kind
        mp_lexer_t *lex;
        if (source_kind == LEX_SRC_STR) {
            const char *line = source;
            lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, line, strlen(line), false);
        } else if (source_kind == LEX_SRC_VSTR) {
            const vstr_t *vstr = source;
            lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, vstr->buf, vstr->len, false);
        } else if (source_kind == LEX_SRC_FILENAME) {
            const char *filename = (const char *)source;
            lex = mp_lexer_new_from_file(qstr_from_str(filename));
        } else { // LEX_SRC_STDIN
            lex = mp_lexer_new_from_fd(MP_QSTR__lt_stdin_gt_, 0, false);
        }

        qstr source_name = lex->source_name;

        #if MICROPY_PY___FILE__
        if (input_kind == MP_PARSE_FILE_INPUT) {
            mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(source_name));
        }
        #endif

        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);

        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, is_repl);

        if (!compile_only) {
            // execute it
            mp_call_function_0(module_fun);
        }

        mp_hal_set_interrupt_char(-1);
        mp_handle_pending(true);
        nlr_pop();
        return 0;

    } else {
        // uncaught exception
        mp_hal_set_interrupt_char(-1);
        mp_handle_pending(false);
        return handle_uncaught_exception(nlr.ret_val);
    }
}

#if MICROPY_USE_READLINE == 1
#include "shared/readline/readline.h"
#endif

static int do_file(const char *file) {
    return execute_from_lexer(LEX_SRC_FILENAME, file, MP_PARSE_FILE_INPUT, false);
}

static int do_str(const char *str) {
    return execute_from_lexer(LEX_SRC_STR, str, MP_PARSE_FILE_INPUT, false);
}

static void print_help(char **argv) {
    cprintf(
        "usage: %s [<opts>] [-X <implopt>] [-c <command> | -m <module> | <filename>]\n"
        "Options:\n"
        "-h : print this help message\n"
        #if MICROPY_DEBUG_PRINTERS
        "-v : verbose (trace various operations); can be multiple\n"
        #endif
        "-O[N] : apply bytecode optimizations of level N\n"
        "\n"
        "Implementation specific options (-X):\n", argv[0]
        );
    int impl_opts_cnt = 0;
    cprintf(
        "  compile-only                 -- parse and compile only\n"
        #if MICROPY_EMIT_NATIVE
        "  emit={bytecode,native,viper} -- set the default code emitter\n"
        #else
        "  emit=bytecode                -- set the default code emitter\n"
        #endif
        );
    impl_opts_cnt++;
    #if MICROPY_ENABLE_GC
    cprintf(
        "  heapsize=<n>[w][K|M] -- set the heap size for the GC (default %ld)\n"
        , heap_size);
    impl_opts_cnt++;
    #endif

    if (impl_opts_cnt == 0) {
        cprintf("  (none)\n");
    }
}

static void invalid_args(void) {
    // TODO: replace with fprintf(stderr)
    cprintf("Invalid command line arguments. Use -h option for help.\n");
}

// Process options which set interpreter init options
static void pre_process_options(int argc, char **argv) {
    for (int a = 1; a < argc; a++) {
        if (argv[a][0] == '-') {
            if (strcmp(argv[a], "-c") == 0 || strcmp(argv[a], "-m") == 0) {
                break; // Everything after this is a command/module and arguments for it
            }
            if (strcmp(argv[a], "-h") == 0) {
                print_help(argv);
                exit(0);
            }
            if (strcmp(argv[a], "-X") == 0) {
                if (a + 1 >= argc) {
                    invalid_args();
                    exit(1);
                }
                if (0) {
                } else if (strcmp(argv[a + 1], "compile-only") == 0) {
                    compile_only = true;
                } else if (strcmp(argv[a + 1], "emit=bytecode") == 0) {
                    emit_opt = MP_EMIT_OPT_BYTECODE;
                #if MICROPY_EMIT_NATIVE
                } else if (strcmp(argv[a + 1], "emit=native") == 0) {
                    emit_opt = MP_EMIT_OPT_NATIVE_PYTHON;
                } else if (strcmp(argv[a + 1], "emit=viper") == 0) {
                    emit_opt = MP_EMIT_OPT_VIPER;
                #endif
                #if MICROPY_ENABLE_GC
                } else if (strncmp(argv[a + 1], "heapsize=", sizeof("heapsize=") - 1) == 0) {
                    char *end;
                    heap_size = strtol(argv[a + 1] + sizeof("heapsize=") - 1, &end, 0);
                    // Don't bring unneeded libc dependencies like tolower()
                    // If there's 'w' immediately after number, adjust it for
                    // target word size. Note that it should be *before* size
                    // suffix like K or M, to avoid confusion with kilowords,
                    // etc. the size is still in bytes, just can be adjusted
                    // for word size (taking 32bit as baseline).
                    bool word_adjust = false;
                    if ((*end | 0x20) == 'w') {
                        word_adjust = true;
                        end++;
                    }
                    if ((*end | 0x20) == 'k') {
                        heap_size *= 1024;
                    } else if ((*end | 0x20) == 'm') {
                        heap_size *= 1024 * 1024;
                    } else {
                        // Compensate for ++ below
                        --end;
                    }
                    if (*++end != 0) {
                        goto invalid_arg;
                    }
                    if (word_adjust) {
                        heap_size = heap_size * MP_BYTES_PER_OBJ_WORD / 4;
                    }
                    // If requested size too small, we'll crash anyway
                    if (heap_size < 700) {
                        goto invalid_arg;
                    }
                #endif
                } else {
                invalid_arg:
                    invalid_args();
                    exit(1);
                }
                a++;
            }
        } else {
            break; // Not an option but a file
        }
    }
}

static void set_sys_argv(char *argv[], int argc, int start_arg) {
    for (int i = start_arg; i < argc; i++) {
        mp_obj_list_append(mp_sys_argv, MP_OBJ_NEW_QSTR(qstr_from_str(argv[i])));
    }
}

#if MICROPY_PY_SYS_EXECUTABLE
extern mp_obj_str_t mp_sys_executable_obj;
static char executable_path[MICROPY_ALLOC_PATH_MAX];

static void sys_set_excecutable(char *argv0) {
    if (realpath(argv0, executable_path)) {
        mp_obj_str_set_data(&mp_sys_executable_obj, (byte *)executable_path, strlen(executable_path));
    }
}
#endif

#ifdef _WIN32
#define PATHLIST_SEP_CHAR ';'
#else
#define PATHLIST_SEP_CHAR ':'
#endif

MP_NOINLINE int main_(int argc, char **argv);

void umain(int argc, char **argv) {
    #if MICROPY_PY_THREAD
    mp_thread_init();
    #endif
    // We should capture stack top ASAP after start, and it should be
    // captured guaranteedly before any other stack variables are allocated.
    // For this, actual main (renamed main_) should not be inlined into
    // this function. main_() itself may have other functions inlined (with
    // their own stack variables), that's why we need this main/main_ split.
    mp_stack_ctrl_init();
    int ret_code = main_(argc, argv);
    #if MICROPY_DEBUG_PRINTERS
    if (mp_verbose_flag) {
        cprintf("python3 return code: %d\n", ret_code);
    }
    #else
    (void)ret_code;
    #endif  /* MICROPY_DEBUG_PRINTERS */
}

MP_NOINLINE int main_(int argc, char **argv) {
    #ifdef SIGPIPE
    // Do not raise SIGPIPE, instead return EPIPE. Otherwise, e.g. writing
    // to peer-closed socket will lead to sudden termination of MicroPython
    // process. SIGPIPE is particularly nasty, because unix shell doesn't
    // print anything for it, so the above looks like completely sudden and
    // silent termination for unknown reason. Ignoring SIGPIPE is also what
    // CPython does. Note that this may lead to problems using MicroPython
    // scripts as pipe filters, but again, that's what CPython does. So,
    // scripts which want to follow unix shell pipe semantics (where SIGPIPE
    // means "pipe was requested to terminate, it's not an error"), should
    // catch EPIPE themselves.
    signal(SIGPIPE, SIG_IGN);
    #endif

    // Define a reasonable stack limit to detect stack overflow.
    mp_uint_t stack_limit = 40000 * (sizeof(void *) / 4);
    mp_stack_set_limit(stack_limit);

    pre_process_options(argc, argv);

    #if MICROPY_ENABLE_GC
    #if !MICROPY_GC_SPLIT_HEAP

    heap_size = ROUNDUP(heap_size, PAGE_SIZE);
    if (heap_size > HUGE_PAGE_SIZE) {
        cprintf("[Error] Python heap size must be limited to HUGE_PAGE_SIZE (%lld), but got %ld\n", HUGE_PAGE_SIZE, heap_size);
    }
    char *heap = (char *)UTEMP + HUGE_PAGE_SIZE;

    int err = sys_alloc_region(CURENVID, heap, heap_size, PROT_RW);
    if (err != 0) {
        cprintf("[Error] Python failed to allocate heap region\n");
        // Failed to allocate heap.
        return err;
    }

    gc_init(heap, heap + heap_size);

    #endif
    #endif

    mp_init();

    #if MICROPY_EMIT_NATIVE
    // Set default emitter options
    MP_STATE_VM(default_emit_opt) = emit_opt;
    #else
    (void)emit_opt;
    #endif

    mp_sys_path = mp_obj_new_list(0, NULL);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));

    mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_argv), 0);

    #if MICROPY_PY_SYS_EXECUTABLE
    sys_set_excecutable(argv[0]);
    #endif

    const int NOTHING_EXECUTED = -2;
    int ret = NOTHING_EXECUTED;
    for (int a = 1; a < argc; a++) {
        if (argv[a][0] == '-') {
            if (strcmp(argv[a], "-c") == 0) {
                if (a + 1 >= argc) {
                    invalid_args();
                    return 1;
                }
                set_sys_argv(argv, a + 1, a); // The -c becomes first item of sys.argv, as in CPython
                set_sys_argv(argv, argc, a + 2); // Then what comes after the command
                ret = do_str(argv[a + 1]);
                break;
            } else if (strcmp(argv[a], "-X") == 0) {
                a += 1;
            #if MICROPY_DEBUG_PRINTERS
            } else if (strcmp(argv[a], "-v") == 0) {
                mp_verbose_flag++;
            #endif
            } else if (strncmp(argv[a], "-O", 2) == 0) {
                if (unichar_isdigit(argv[a][2])) {
                    MP_STATE_VM(mp_optimise_value) = argv[a][2] & 0xf;
                } else {
                    MP_STATE_VM(mp_optimise_value) = 0;
                    for (char *p = argv[a] + 1; *p && *p == 'O'; p++, MP_STATE_VM(mp_optimise_value)++) {;
                    }
                }
            } else {
                invalid_args();
                return 1;
            }
        } else {
            // char pathbuf[MAXPATHLEN];
            // char *basedir = realpath(argv[a], pathbuf);
            char *basedir = argv[a];

            // Set base dir of the script as first entry in sys.path.
            char *p = strrchr(basedir, '/');
            const char *base_name = "";
            size_t len = 0;
            if (p != NULL) {
                base_name = basedir;
                len = p - basedir;
            }
            mp_obj_list_store(mp_sys_path, MP_OBJ_NEW_SMALL_INT(0), mp_obj_new_str_via_qstr(base_name, len));

            set_sys_argv(argv, argc, a);
            ret = do_file(argv[a]);
            break;
        }
    }

    if (ret == NOTHING_EXECUTED) {
        ret = execute_from_lexer(LEX_SRC_STDIN, NULL, MP_PARSE_FILE_INPUT, false);
    }

    #if MICROPY_PY_SYS_SETTRACE
    MP_STATE_THREAD(prof_trace_callback) = MP_OBJ_NULL;
    #endif

    #if MICROPY_PY_SYS_ATEXIT
    // Beware, the sys.settrace callback should be disabled before running sys.atexit.
    if (mp_obj_is_callable(MP_STATE_VM(sys_exitfunc))) {
        mp_call_function_0(MP_STATE_VM(sys_exitfunc));
    }
    #endif

    #if MICROPY_PY_MICROPYTHON_MEM_INFO && MICROPY_DEBUG_PRINTERS
    if (mp_verbose_flag) {
        mp_micropython_mem_info(0, NULL);
    }
    #endif /* MICROPY_PY_MICROPYTHON_MEM_INFO && MICROPY_DEBUG_PRINTERS */

    #if MICROPY_PY_BLUETOOTH
    void mp_bluetooth_deinit(void);
    mp_bluetooth_deinit();
    #endif

    #if MICROPY_PY_THREAD
    mp_thread_deinit();
    #endif

    #if defined(MICROPY_UNIX_COVERAGE)
    gc_sweep_all();
    #endif

    mp_deinit();

    #if MICROPY_ENABLE_GC && !defined(NDEBUG)
    // We don't really need to free memory since we are about to exit the
    // process, but doing so helps to find memory leaks.
    #if !MICROPY_GC_SPLIT_HEAP
    sys_unmap_region(CURENVID, heap, heap_size);
    #else
    for (size_t i = 0; i < MICROPY_GC_SPLIT_HEAP_N_HEAPS; i++) {
        free(heaps[i]);
    }
    #endif
    #endif

    return ret & 0xff;
}
