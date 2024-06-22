/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// Options to control how MicroPython is built for this port, overriding
// defaults in py/mpconfig.h. This file is mostly about configuring the
// features to work on Unix-like systems, see mpconfigvariant.h (and
// mpconfigvariant_common.h) for feature enabling.

#include <lib.h>
#include <random.h>

// Use JOS print
#define MICROPY_USE_INTERNAL_PRINTF         (0)

// Send raise KeyboardInterrupt directly from the signal handler rather than
// scheduling it into the VM.
#define MICROPY_ASYNC_KBD_INTR         (0)

// Enable helpers for printing debugging information.
#ifndef MICROPY_DEBUG_PRINTERS
#define MICROPY_DEBUG_PRINTERS         (0)
#endif

// Enable floating point by default.
// TODO: uncomment if floating-point registers are supported in QEMU.
// #ifndef MICROPY_FLOAT_IMPL
// #define MICROPY_FLOAT_IMPL             (MICROPY_FLOAT_IMPL_DOUBLE)
// #endif

// Don't use native _Float16 because it increases code size by a lot.
#ifndef MICROPY_FLOAT_USE_NATIVE_FLT16
#define MICROPY_FLOAT_USE_NATIVE_FLT16 (0)
#endif

// Enable arbitrary precision long-int by default.
#ifndef MICROPY_LONGINT_IMPL
#define MICROPY_LONGINT_IMPL           (MICROPY_LONGINT_IMPL_MPZ)
#endif

// Enable use of C libraries that need read/write/lseek/fsync, e.g. axtls.
#define MICROPY_STREAMS_POSIX_API      (0)

// REPL conveniences.
#define MICROPY_REPL_EMACS_WORDS_MOVE  (1)
#define MICROPY_REPL_EMACS_EXTRA_WORDS_MOVE (1)
#define MICROPY_USE_READLINE_HISTORY   (0)
// #ifndef MICROPY_READLINE_HISTORY_SIZE
// #define MICROPY_READLINE_HISTORY_SIZE  (50)
// #endif

// Seed random on import.
#define MICROPY_PY_RANDOM_SEED_INIT_FUNC (mp_random_seed_init())

// Allow exception details in low-memory conditions.
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
#define MICROPY_EMERGENCY_EXCEPTION_BUF_SIZE (256)

// Allow loading of .mpy files.
#define MICROPY_PERSISTENT_CODE_LOAD   (1)

// Extra memory debugging.
#define MICROPY_MALLOC_USES_ALLOCATED_SIZE (1)
#define MICROPY_MEM_STATS              (1)

// Enable a small performance boost for the VM.
#define MICROPY_OPT_COMPUTED_GOTO      (1)

// Return number of collected objects from gc.collect().
#define MICROPY_PY_GC_COLLECT_RETVAL   (1)

// Enable detailed error messages and warnings.
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_DETAILED)
#define MICROPY_WARNINGS               (1)
#define MICROPY_PY_STR_BYTES_CMP_WARN  (1)

// Configure the "sys" module with features not usually enabled on bare-metal.
#define MICROPY_PY_SYS_ATEXIT          (1)
#define MICROPY_PY_SYS_EXC_INFO        (1)

// Configure the "os" module with extra unix features.
#define MICROPY_PY_OS_INCLUDEFILE               "ports/jos/modos.c"
#define MICROPY_PY_OS_ERRNO                     (0)
#define MICROPY_PY_OS_GETENV_PUTENV_UNSETENV    (0)
#define MICROPY_PY_OS_SEP                       (0)
#define MICROPY_PY_OS_SYSTEM                    (1)
#define MICROPY_PY_OS_URANDOM                   (1)
#define MICROPY_PY_OS_UNAME                     (0)

// Enable the unix-specific "time" module.
#define MICROPY_PY_TIME                (1)
#define MICROPY_PY_TIME_TIME_TIME_NS   (1)
#define MICROPY_PY_TIME_CUSTOM_SLEEP   (1)
#define MICROPY_PY_TIME_INCLUDEFILE    "ports/jos/modtime.c"

// The "select" module is enabled by default, but disable select.select().
#define MICROPY_PY_SELECT_POSIX_OPTIMISATIONS (0)
#define MICROPY_PY_SELECT_SELECT       (0)

// Enable the "websocket" module.
#define MICROPY_PY_WEBSOCKET           (0)

// Enable the "machine" module, mostly for machine.mem*.
#define MICROPY_PY_MACHINE             (1)
#define MICROPY_PY_MACHINE_PULSE       (1)
#define MICROPY_PY_MACHINE_PIN_BASE    (1)

#define MICROPY_USE_INTERNAL_ERRNO                  (1)

#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)

// Disable native emitters.
#define MICROPY_EMIT_X86 (0)
#define MICROPY_EMIT_X64 (0)
#define MICROPY_EMIT_THUMB (0)
#define MICROPY_EMIT_ARM (0)

// Tune the parser to use less RAM by default.
#define MICROPY_ALLOC_QSTR_CHUNK_INIT (64)
#define MICROPY_ALLOC_PARSE_RULE_INIT (8)
#define MICROPY_ALLOC_PARSE_RULE_INC  (8)
#define MICROPY_ALLOC_PARSE_RESULT_INIT (8)
#define MICROPY_ALLOC_PARSE_RESULT_INC (8)
#define MICROPY_ALLOC_PARSE_CHUNK_INIT (64)

// Enable features that are not enabled by default with the minimum config.
#define MICROPY_COMP_CONST_FOLDING (1)
#define MICROPY_COMP_CONST_LITERAL (1)
#define MICROPY_COMP_CONST_TUPLE (1)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (1)
#define MICROPY_ENABLE_COMPILER (1)
#define MICROPY_ENABLE_EXTERNAL_IMPORT (1)
#define MICROPY_FULL_CHECKS (1)
#define MICROPY_HELPER_REPL (1)
#define MICROPY_KBD_EXCEPTION (1)
#define MICROPY_MODULE_GETATTR (1)
#define MICROPY_MULTIPLE_INHERITANCE (1)
#define MICROPY_PY_ASSIGN_EXPR (1)
#define MICROPY_PY_ASYNC_AWAIT (1)
#define MICROPY_PY_ATTRTUPLE (1)
#define MICROPY_PY_BUILTINS_DICT_FROMKEYS (1)
#define MICROPY_PY_BUILTINS_RANGE_ATTRS (1)
#define MICROPY_PY_GENERATOR_PEND_THROW (1)

// Enable just the sys and os built-in modules.
#define MICROPY_PY_SYS (1)
#define MICROPY_PY_OS (1)
#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)


// Disable native emitters.
#define MICROPY_EMIT_X86 (0)
#define MICROPY_EMIT_X64 (0)
#define MICROPY_EMIT_THUMB (0)
#define MICROPY_EMIT_ARM (0)

// Tune the parser to use less RAM by default.
#define MICROPY_ALLOC_QSTR_CHUNK_INIT (64)
#define MICROPY_ALLOC_PARSE_RULE_INIT (8)
#define MICROPY_ALLOC_PARSE_RULE_INC  (8)
#define MICROPY_ALLOC_PARSE_RESULT_INIT (8)
#define MICROPY_ALLOC_PARSE_RESULT_INC (8)
#define MICROPY_ALLOC_PARSE_CHUNK_INIT (64)

// Enable features that are not enabled by default with the minimum config.
#define MICROPY_COMP_CONST_FOLDING (1)
#define MICROPY_COMP_CONST_LITERAL (1)
#define MICROPY_COMP_CONST_TUPLE (1)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (1)
#define MICROPY_ENABLE_COMPILER (1)
#define MICROPY_ENABLE_EXTERNAL_IMPORT (1)
#define MICROPY_FULL_CHECKS (1)
#define MICROPY_HELPER_REPL (1)
#define MICROPY_KBD_EXCEPTION (1)
#define MICROPY_MODULE_GETATTR (1)
#define MICROPY_MULTIPLE_INHERITANCE (1)
#define MICROPY_PY_ASSIGN_EXPR (1)
#define MICROPY_PY_ASYNC_AWAIT (1)
#define MICROPY_PY_ATTRTUPLE (1)
#define MICROPY_PY_BUILTINS_DICT_FROMKEYS (1)
#define MICROPY_PY_BUILTINS_RANGE_ATTRS (1)
#define MICROPY_PY_GENERATOR_PEND_THROW (1)

// Enable just the sys and os built-in modules.
#define MICROPY_PY_SYS (1)
#define MICROPY_PY_OS (1)


#ifndef MICROPY_CONFIG_ROM_LEVEL
#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_CORE_FEATURES)
#endif

#ifndef MICROPY_PY_SYS_PLATFORM
#define MICROPY_PY_SYS_PLATFORM  "jos"
#endif

#ifndef MICROPY_PY_SYS_PATH_DEFAULT
#define MICROPY_PY_SYS_PATH_DEFAULT ".frozen:~/.micropython/lib:/usr/lib/micropython"
#endif

#define MP_STATE_PORT MP_STATE_VM

// Configure which emitter to use for this target.
#if !defined(MICROPY_EMIT_X64) && defined(__x86_64__)
    #define MICROPY_EMIT_X64        (1)
#endif
#if !defined(MICROPY_EMIT_X86) && defined(__i386__)
    #define MICROPY_EMIT_X86        (1)
#endif

// Type definitions for the specific machine based on the word size.
#ifndef MICROPY_OBJ_REPR
#ifdef __LP64__
typedef long mp_int_t; // must be pointer size
typedef unsigned long mp_uint_t; // must be pointer size
#else
// These are definitions for machines where sizeof(int) == sizeof(void*),
// regardless of actual size.
typedef int mp_int_t; // must be pointer size
typedef unsigned int mp_uint_t; // must be pointer size
#endif
#else
// Assume that if we already defined the obj repr then we also defined types.
#endif

// Cannot include <sys/types.h>, as it may lead to symbol name clashes
#if _FILE_OFFSET_BITS == 64 && !defined(__LP64__)
typedef long long mp_off_t;
#else
typedef long mp_off_t;
#endif

// We need to provide a declaration/definition of alloca()
// unless support for it is disabled.
#if !defined(MICROPY_NO_ALLOCA) || MICROPY_NO_ALLOCA == 0
/* Remove any previous definition.  */
#undef	alloca

/* Allocate a block that will be freed when the calling function exits.  */
extern void *alloca (size_t __size);

#ifdef	__GNUC__
# define alloca(size)	__builtin_alloca (size)
#endif /* GCC.  */
#endif

// Always enable GC.
#define MICROPY_ENABLE_GC           (1)

#if !(defined(MICROPY_GCREGS_SETJMP) || defined(__x86_64__) || defined(__i386__))
// Fall back to setjmp() implementation for discovery of GC pointers in registers.
#define MICROPY_GCREGS_SETJMP (1)
#endif

// VFS stat functions should return time values relative to 1970/1/1
#define MICROPY_EPOCH_IS_1970       (1)

// Assume that select() call, interrupted with a signal, and erroring
// with EINTR, updates remaining timeout value.
#define MICROPY_SELECT_REMAINING_TIME (1)

// Disable stackless by default.
#ifndef MICROPY_STACKLESS
#define MICROPY_STACKLESS           (0)
#define MICROPY_STACKLESS_STRICT    (0)
#endif

// FS support.
#define MICROPY_VFS                 (0)
#define MICROPY_VFS_POSIX           (0)
#define MICROPY_READER_VFS          (0)

// Implementation of the machine module.
#define MICROPY_PY_MACHINE_INCLUDEFILE "ports/jos/modmachine.c"

// Unix-specific configuration of machine.mem*.
#define MICROPY_MACHINE_MEM_GET_READ_ADDR   mod_machine_mem_get_addr
#define MICROPY_MACHINE_MEM_GET_WRITE_ADDR  mod_machine_mem_get_addr

#define MICROPY_ALLOC_PATH_MAX      (MAXPATHLEN)

// Ensure builtinimport.c works with -m.
#define MICROPY_MODULE_OVERRIDE_MAIN_IMPORT (1)

// Don't default sys.argv and sys.path because we do that in main.
#define MICROPY_PY_SYS_PATH_ARGV_DEFAULTS (0)

// Enable sys.executable.
#define MICROPY_PY_SYS_EXECUTABLE (0)

// Bare-metal ports don't have stderr. Printing debug to stderr may give tests
// which check stdout a chance to pass, etc.
extern const struct _mp_print_t mp_stderr_print;
#define MICROPY_DEBUG_PRINTER (&mp_stderr_print)
#define MICROPY_ERROR_PRINTER (&mp_stderr_print)

// For the native emitter configure how to mark a region as executable.
void mp_unix_alloc_exec(size_t min_size, void **ptr, size_t *size);
void mp_unix_free_exec(void *ptr, size_t size);
void mp_unix_mark_exec(void);
#define MP_PLAT_ALLOC_EXEC(min_size, ptr, size) mp_unix_alloc_exec(min_size, ptr, size)
#define MP_PLAT_FREE_EXEC(ptr, size) mp_unix_free_exec(ptr, size)
#ifndef MICROPY_FORCE_PLAT_ALLOC_EXEC
// Use MP_PLAT_ALLOC_EXEC for any executable memory allocation, including for FFI
// (overriding libffi own implementation)
#define MICROPY_FORCE_PLAT_ALLOC_EXEC (1)
#endif

// If enabled, configure how to seed random on init.
#ifdef MICROPY_PY_RANDOM_SEED_INIT_FUNC
void mp_hal_get_random(size_t n, void *buf);

static inline unsigned long mp_random_seed_init(void) {
    unsigned long r;
    mp_hal_get_random(sizeof(r), &r);
    rand_init(r);
    return r;
}
#endif

// From "man readdir": "Under glibc, programs can check for the availability
// of the fields [in struct dirent] not defined in POSIX.1 by testing whether
// the macros [...], _DIRENT_HAVE_D_TYPE are defined."
// Other libc's don't define it, but proactively assume that dirent->d_type
// is available on a modern *nix system.
#ifndef _DIRENT_HAVE_D_TYPE
#define _DIRENT_HAVE_D_TYPE (1)
#endif
// This macro is not provided by glibc but we need it so ports that don't have
// dirent->d_ino can disable the use of this field.
#ifndef _DIRENT_HAVE_D_INO
#define _DIRENT_HAVE_D_INO (1)
#endif

// For debugging purposes, make cprintf() available to any source file.

// Configure the implementation of machine.idle().
#define MICROPY_UNIX_MACHINE_IDLE sys_yield();
