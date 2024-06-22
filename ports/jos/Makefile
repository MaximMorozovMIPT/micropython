LDFLAGS :=
CFLAGS :=
CFLAGS_EXTRA :=

include ../../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

PROG ?= micropython

# qstr definitions (must come before including py.mk)
QSTR_DEFS += qstrdefsport.h

# include py core make definitions
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

# JOS specific settings.
JOS_ROOT := $(TOP)/..
JOS_OBJ_DIR := $(JOS_ROOT)/obj

-include $(JOS_ROOT)/conf/lab.mk

ifdef JOSLLVM

CC	:= $(CLANGPREFIX)clang$(CLANG_VERSION) -target x86_64-gnu-linux -pipe
AS	:= $(CLANGPREFIX)llvm-as$(CLANG_VERSION)
AR	:= $(CLANGPREFIX)llvm-ar$(CLANG_VERSION)
LD	:= $(CLANGPREFIX)ld.lld$(CLANG_VERSION)
OBJCOPY	:= llvm/gnu-objcopy$(CLANG_VERSION)
OBJDUMP	:= $(CLANGPREFIX)llvm-objdump$(CLANG_VERSION)
NM	:= $(CLANGPREFIX)llvm-nm$(CLANG_VERSION)

CFLAGS_EXTRA += -Wno-self-assign -Wno-format-nonliteral -Wno-address-of-packed-member \
                -Wno-frame-address -Wno-unknown-warning-option

LDFLAGS_EXTRA += -fuse-ld=lld
CFLAGS_EXTRA += -nostdlibinc

LIBGCC_FILE_NAME := $(shell A="$$($(CC) $(CFLAGS) -print-resource-dir)/lib"; [ -d "$$A/linux" ] && echo "$$A/linux" || echo "$$A/ispras")/libclang_rt.builtins-x86_64.a

else

CC	:= $(GCCPREFIX)gcc$(GCC_VERSION) -fno-pic -pipe
AS	:= $(GCCPREFIX)as
AR	:= $(GCCPREFIX)ar
LD	:= $(GCCPREFIX)ld
OBJCOPY	:= $(GCCPREFIX)objcopy
OBJDUMP	:= $(GCCPREFIX)objdump
NM	:= $(GCCPREFIX)nm

CFLAGS_EXTRA += -Wno-unused-but-set-variable

LIBGCC_FILE_NAME := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

endif

USER_SAN_CFLAGS :=
USER_SAN_LDFLAGS :=
USER_EXTRA_OBJFILES :=

ifdef UASAN

CFLAGS_EXTRA += -DSAN_ENABLE_UASAN

USER_SAN_CFLAGS := -fsanitize=address -fsanitize-blacklist=$(JOS_ROOT)/llvm/ublacklist.txt
USER_SAN_CFLAGS += -mllvm -asan-globals-live-support=0
USER_SAN_CFLAGS += -mllvm $(shell sed -n 's/^\#define SANITIZE_USER_SHADOW_BASE \(.*\)/ -asan-mapping-offset=\1 /p' $(JOS_ROOT)/inc/memlayout.h)

USER_SAN_LDFLAGS := -Wl,--wrap,memcpy  \
	-Wl,--wrap,memset  \
	-Wl,--wrap,memmove \
	-Wl,--wrap,bcopy   \
	-Wl,--wrap,bzero   \
	-Wl,--wrap,bcmp    \
	-Wl,--wrap,memcmp  \
	-Wl,--wrap,strcat  \
	-Wl,--wrap,strcpy  \
	-Wl,--wrap,strlcpy \
	-Wl,--wrap,strncpy \
	-Wl,--wrap,strlcat \
	-Wl,--wrap,strncat \
	-Wl,--wrap,strnlen \
	-Wl,--wrap,strlen

USER_EXTRA_OBJFILES := \
	$(JOS_OBJ_DIR)/user/asan/asan.o \
	$(JOS_OBJ_DIR)/user/asan/asan_internal.o \
	$(JOS_OBJ_DIR)/user/asan/asan_memintrinsics.o \
	$(JOS_OBJ_DIR)/user/asan/asan_platform.ujos.o \
	$(JOS_OBJ_DIR)/user/asan/asan_prim.o \
	$(JOS_OBJ_DIR)/user/asan/asan_used.o

endif

ifdef UUBSAN

CFLAGS_EXTRA += -DSAN_ENABLE_UUBSAN

USER_SAN_CFLAGS += -fsanitize=undefined \
	-fsanitize=implicit-integer-truncation \
	-fno-sanitize=function \
	-fno-sanitize=vptr \
	-fno-sanitize=return

USER_EXTRA_OBJFILES += \
	$(JOS_OBJ_DIR)/user/ubsan/ubsan.o

endif

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

# Include JOS source tree.
INC += -I$(JOS_ROOT)/
INC += -I$(JOS_ROOT)/inc/

# Debugging/Optimization
COPT ?= -Og

# compiler settings
CFLAGS += $(INC) $(CWARN) -std=gnu99 -nostdlib $(COPT) $(CFLAGS_EXTRA) $(USER_SAN_CFLAGS)

# Note: Symbols and debug information will still be stripped from the final binary
# unless "DEBUG=1" or "STRIP=" is passed to make, see README.md for details.
CFLAGS += -g -O1

# -fno-builtin is required to avoid refs to undefined functions in the kernel.
CFLAGS += -fno-builtin -MD -ffreestanding -fno-omit-frame-pointer -mno-red-zone
CFLAGS += -Wall -Wformat=2 -Wno-unused-function -Werror -g -gpubnames -gdwarf-4
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
# No FP registers.
CFLAGS += -mno-sse -mno-sse2 -mno-mmx
CFLAGS += -mcmodel=large -m64
CFLAGS += -DJOS_USER -DLAB=12
CFLAGS += -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables

# source files
SRC_C += \
	gccollect.c \
	input.c \
	main.c \
	mphalport.c \
	modio.c \
	lexer.c


SRC_O += $(USER_EXTRA_OBJFILES)

SHARED_SRC_C += $(addprefix shared/,\
	runtime/gchelper_generic.c \
	timeutils/timeutils.c \
	$(SHARED_SRC_C_EXTRA) \
	)

OBJ += $(PY_O)
OBJ += $(JOS_OBJ_DIR)/lib/entry.o
OBJ += $(USER_EXTRA_OBJFILES)
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SHARED_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))


ifneq ($(FROZEN_MANIFEST),)
# force 16 bits to work on both 32 and 64 bit archs
CFLAGS += -DMPZ_DIG_SIZE=16
endif

LIBS =

LDFLAGS += $(LDFLAGS_MOD) $(LDFLAGS_ARCH) $(LDFLAGS_EXTRA) $(USER_SAN_LDFLAGS)

# Linker flags for JOS programs
USER_LD_SCRIPT = $(TOP)/../user/user.ld
LDFLAGS += -Wl,-T,$(USER_LD_SCRIPT)

LDFLAGS += -Wl,-m,elf_x86_64
LDFLAGS += -Wl,-z,max-page-size=0x1000
LDFLAGS += -Wl,-z,noexecstack

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--warn-common
LDFLAGS += -Wl,--build-id=none
LDFLAGS += -Wl,-nostdlib
LDFLAGS += -nostdlib
LDFLAGS += -Wl,-Map=$@.map,--cref
LDFLAGS += -Wl,-no-pie

LDFLAGS += -Wl,$(JOS_OBJ_DIR)/lib/libjos.a

LDFLAGS += -Wl,$(LIBGCC_FILE_NAME)

LDFLAGS += -Wl,-static

STRIPFLAGS_EXTRA += $(BUILD)/$(PROG) -R .eh_frame -R .eh_frame_hdr

include $(TOP)/py/mkrules.mk
