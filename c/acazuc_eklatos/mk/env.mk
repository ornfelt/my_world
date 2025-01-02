include $(ROOTDIR)/config.mk

export ARCH

CC_BIN_DIR = ${ROOTDIR}/cc/build/bin
CC = $(CC_BIN_DIR)/$(HOST)-gcc
CXX = $(CC_BIN_DIR)/$(HOST)-g++
LD = $(CC_BIN_DIR)/$(HOST)-ld
OBJCOPY = $(CC_BIN_DIR)/$(HOST)-objcopy
STRIP = $(CC_BIN_DIR)/$(HOST)-strip
MKDIR = mkdir
TOUCH = touch
ECHO = echo
TAR = tar
CP = cp
LN = ln
RM = rm
MV = mv
PATCH = patch
SH = sh

IDQUOTE = $(addprefix -iquote , $(SRC_PATH))

CFLAGS+= -O2 \
         -Wall \
         -Wextra \
         -Wshadow \
         -Wpointer-arith \
         -g \

CXXFLAGS+= -O2 \
           -Wall \
           -Wextra \
           -Wshadow \
           -Wpointer-arith \
           -g \

#CFLAGS+= -fanalyzer

BUILDDIR = $(ROOTDIR)/build/$(ARCH)

OBJDIR = $(ROOTDIR)/obj/$(ARCH)

HARDENING_CFLAGS = -fPIC \
                   -D_FORTIFY_SOURCE=3 \
                   -ftrivial-auto-var-init=pattern \
                   -fstack-clash-protection \
                   -fno-plt \

HARDENING_CXXFLAGS = -fPIC \
                     -D_FORTIFY_SOURCE=3 \
                     -fstack-clash-protection \
                     -fno-plt \

HARDENING_LDFLAGS = -fPIC \

ifeq ($(KERNEL), yes)

CFLAGS+= -ffreestanding \
         -fno-builtin \
         -nostdinc \
         -fno-stack-protector \
         -fcf-protection=none \

LDFLAGS+= -nostdlib \
          -nodefaultlibs \
          -nostartfiles \

else

CFLAGS+= $(HARDENING_CFLAGS)
CXXFLAGS+= $(HARDENING_CXXFLAGS)
LDFLAGS+= $(HARDENING_LDFLAGS)

endif


ifeq ($(ARCH), i386)

HOSTPREFIX = i686
HOST = i686-unknown-eklat
LDMODE = -m elf_i386
ARCH_SYSDIR = i386

ifeq ($(KERNEL), yes)

WITH_ACPI = yes
CFLAGS+= -mgeneral-regs-only

else

FPU_FLAGS+= -mfpmath=sse

ifeq ($(WITH_SSE), yes)
FPU_FLAGS+= -msse
endif
ifeq ($(WITH_SSE2), yes)
FPU_FLAGS+= -msse2
endif
ifeq ($(WITH_SSE3), yes)
FPU_FLAGS+= -msse3
endif
ifeq ($(WITH_SSSE3), yes)
FPU_FLAGS+= -mssse3
endif
ifeq ($(WITH_SSE41), yes)
FPU_FLAGS+= -msse4.1
endif
ifeq ($(WITH_SSE42), yes)
FPU_FLAGS+= -msse4.2
endif

CFLAGS+= $(FPU_FLAGS)
CXXFLAGS+= $(FPU_FLAGS)

endif

endif

ifeq ($(ARCH), amd64)

HOSTPREFIX = x86_64
HOST = x86_64-unknown-eklat
LDMODE = -m elf_x86_64
ARCH_SYSDIR = amd64

ifeq ($(KERNEL), yes)

WITH_ACPI = yes
CFLAGS+= -mgeneral-regs-only

else

FPU_FLAGS+= -mfpmath=sse

ifeq ($(WITH_SSE), yes)
FPU_FLAGS+= -msse
endif
ifeq ($(WITH_SSE2), yes)
FPU_FLAGS+= -msse2
endif
ifeq ($(WITH_SSE3), yes)
FPU_FLAGS+= -msse3
endif
ifeq ($(WITH_SSSE3), yes)
FPU_FLAGS+= -mssse3
endif
ifeq ($(WITH_SSE41), yes)
FPU_FLAGS+= -msse4.1
endif
ifeq ($(WITH_SSE42), yes)
FPU_FLAGS+= -msse4.2
endif
ifeq ($(WITH_AVX), yes)
FPU_FLAGS+= -mavx
endif
ifeq ($(WITH_AVX2), yes)
FPU_FLAGS+= -mavx2
endif

CFLAGS+= $(FPU_FLAGS)
CXXFLAGS+= $(FPU_FLAGS)

endif

endif

ifeq ($(ARCH), aarch64)

HOSTPREFIX = aarch64
HOST = aarch64-unknown-eklat
LDMODE = -m aarch64eklat
CFLAGS+= -mtls-dialect=trad
ARCH_SYSDIR = aarch64

ifeq ($(KERNEL), yes)
WITH_ACPI = yes
CFLAGS+= -mgeneral-regs-only \
         -mtp=tpidr_el1
endif

endif

ifeq ($(ARCH), arm)

HOSTPREFIX = arm
HOST = arm-unknown-eklateabi
LDMODE = -m armelf
CFLAGS+= -fno-omit-frame-pointer
ARCH_SYSDIR = arm

ifeq ($(KERNEL), yes)
WITH_FDT = yes
#XXX -mno-unaligned-access is a little bit overkill, we should
#found a better way to fix this things
CFLAGS+= -mgeneral-regs-only \
         -mtp=tpidrprw \
         -mno-unaligned-access
endif

endif

ifeq ($(ARCH), riscv32)

HOSTPREFIX = riscv32
HOST = riscv32-unknown-eklat
LDMODE = -m elf32lriscv
CFLAGS+= -fno-omit-frame-pointer
HARDENING_CFLAGS += -mno-plt
HARDENING_CXXFLAGS += -mno-plt
ARCH_SYSDIR = riscv

ifeq ($(KERNEL), yes)
WITH_FDT = yes
endif

endif

ifeq ($(ARCH), riscv64)

HOSTPREFIX = riscv64
HOST = riscv64-unknown-eklat
LDMODE = -m elf64lriscv
CFLAGS+= -fno-omit-frame-pointer
HARDENING_CFLAGS += -mno-plt
HARDENING_CXXFLAGS += -mno-plt
ARCH_SYSDIR = riscv

ifeq ($(KERNEL), yes)
WITH_FDT = yes
endif

endif

ifeq ($(ARCH), mipsel)

HOSTPREFIX = mipsel
HOST = mipsel-unknown-eklat
LDMODE = -m elf32ltsmip
CFLAGS+= -fno-omit-frame-pointer
ARCH_SYSDIR = mips

ifeq ($(KERNEL), yes)
WITH_FDT = yes
endif

endif

ifeq ($(ARCH), mips64el)

HOSTPREFIX = mips64el
HOST = mips64el-unknown-eklat
LDMODE = -m elf64ltsmip
CFLAGS+= -fno-omit-frame-pointer
ARCH_SYSDIR = mips

ifeq ($(KERNEL), yes)
WITH_FDT = yes
endif

endif


ifeq ($(KERNEL), yes)
ifeq ($(WITH_ACPI), yes)
CPPFLAGS+= -DWITH_ACPI
endif
ifeq ($(WITH_FDT), yes)
CPPFLAGS+= -DWITH_FDT
endif
endif
