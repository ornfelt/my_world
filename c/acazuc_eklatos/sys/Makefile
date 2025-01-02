KERNEL = yes

include $(MAKEDIR)/env.mk

BIN = $(OBJ_PATH)/eklatos.bin

BIN_LN = eklatos.bin

LDFILE = arch/$(ARCH_SYSDIR)/linker.ld

KSTACK_SIZE = 1024 * 1024

ARCH_DIR = $(OBJ_PATH)/include/arch

SRC = fs/vfs.c \
      fs/mbr.c \
      fs/gpt.c \
      fs/devfs/devfs.c \
      fs/ramfs/ramfs.c \
      fs/tarfs/tarfs.c \
      fs/sysfs/sysfs.c \
      fs/procfs/procfs.c \
      kern/sched.c \
      kern/proc.c \
      kern/file.c \
      kern/mutex.c \
      kern/waitq.c \
      kern/rwlock.c \
      kern/sock.c \
      kern/pipe.c \
      kern/ringbuf.c \
      kern/time.c \
      kern/sma.c \
      kern/dev.c \
      kern/random.c \
      kern/syscall.c \
      kern/disk.c \
      kern/uio.c \
      kern/tty.c \
      kern/ipc.c \
      kern/ramfile.c \
      kern/pipebuf.c \
      kern/poll.c \
      kern/pty.c \
      kern/multiboot.c \
      kern/elf.c \
      kern/kmod.c \
      kern/irq.c \
      kern/ksym.c \
      kern/cpu.c \
      kern/fb.c \
      kern/vtty.c \
      kern/font8x8.c \
      kern/efi.c \
      kern/printf.c \
      kern/ctype.c \
      kern/string.c \
      kern/malloc.c \
      kern/stdlib.c \
      kern/timer.c \
      kern/sg.c \
      net/if.c \
      net/ether.c \
      net/pkt.c \
      net/loopback.c \
      net/raw.c \
      net/ip6.c \
      net/ip4.c \
      net/arp.c \
      net/local/local.c \
      net/local/stream.c \
      net/local/dgram.c \
      mem/region.c \
      mem/pm.c \
      mem/copy.c \
      mem/space.c \
      mem/dma.c \

ifeq ($(ARCH), i386)

SRC+= arch/i386/boot.S \
      arch/i386/i386.c \
      arch/i386/gdt.c \
      arch/i386/gdt.S \
      arch/i386/idt.c \
      arch/i386/trap.S \
      arch/i386/mem.c \
      arch/i386/smp.S \
      arch/i386/wakeup.S \
      arch/i386/wait.S \
      arch/x86/cpuid.c \
      arch/x86/mptable.c \
      arch/x86/x86.c \
      arch/x86/ioapic.c \
      arch/x86/lapic.c \
      arch/x86/rtc.c \
      arch/x86/pic.c \
      arch/x86/pit.c \
      arch/x86/com.c \
      arch/x86/irq.c \
      arch/x86/tsc.c \
      arch/x86/fpu.c \
      arch/x86/hpet.c \
      arch/x86/vga.c \

endif

ifeq ($(ARCH), amd64)

SRC+= arch/amd64/boot.S \
      arch/amd64/amd64.c \
      arch/amd64/mem.c \
      arch/amd64/gdt.c \
      arch/amd64/gdt.S \
      arch/amd64/idt.c \
      arch/amd64/trap.S \
      arch/amd64/smp.S \
      arch/amd64/wakeup.S \
      arch/amd64/wait.S \
      arch/x86/cpuid.c \
      arch/x86/mptable.c \
      arch/x86/x86.c \
      arch/x86/ioapic.c \
      arch/x86/lapic.c \
      arch/x86/rtc.c \
      arch/x86/pic.c \
      arch/x86/pit.c \
      arch/x86/com.c \
      arch/x86/irq.c \
      arch/x86/tsc.c \
      arch/x86/fpu.c \
      arch/x86/hpet.c \
      arch/x86/vga.c \

CFLAGS+= -mcmodel=kernel \
         -mno-red-zone \

endif

ifeq ($(ARCH), arm)

SRC += arch/arm/arm.c \
       arch/arm/mem.c \
       arch/arm/irq.c \
       arch/arm/boot.S \
       arch/arm/trap.S \
       arch/arm/smp.S \
       arch/arm/wait.S \
       arch/aarch64/pl031.c \
       arch/aarch64/pl011.c \
       arch/aarch64/gicv2.c \
       arch/aarch64/timer.c \
       arch/aarch64/psci.c \

endif

ifeq ($(ARCH), aarch64)

SRC += arch/aarch64/aarch64.c \
       arch/aarch64/mem.c \
       arch/aarch64/irq.c \
       arch/aarch64/boot.S \
       arch/aarch64/wakeup.S \
       arch/aarch64/pl031.c \
       arch/aarch64/fpu.S \
       arch/aarch64/pl011.c \
       arch/aarch64/gicv2.c \
       arch/aarch64/trap.S \
       arch/aarch64/timer.c \
       arch/aarch64/psci.c \
       arch/aarch64/smp.S \
       arch/aarch64/wait.S \

endif

ifeq ($(ARCH), riscv32)

LDFILE = arch/riscv/linker32.ld

SRC += arch/riscv/riscv.c \
       arch/riscv/mem32.c \
       arch/riscv/irq.c \
       arch/riscv/boot32.S \
       arch/riscv/fpu.S \
       arch/riscv/timer.c \
       arch/riscv/rtc.c \
       arch/riscv/uart.c \
       arch/riscv/trap.S \
       arch/riscv/clint.c \
       arch/riscv/plic.c \
       arch/riscv/wait.S \
       arch/riscv/syscon.c \
       arch/riscv/sbi.c \
       arch/riscv/smp.S \

CFLAGS+= -mcmodel=medany \

endif

ifeq ($(ARCH), riscv64)

LDFILE = arch/riscv/linker64.ld

SRC += arch/riscv/riscv.c \
       arch/riscv/mem64.c \
       arch/riscv/irq.c \
       arch/riscv/boot64.S \
       arch/riscv/fpu.S \
       arch/riscv/timer.c \
       arch/riscv/rtc.c \
       arch/riscv/uart.c \
       arch/riscv/trap.S \
       arch/riscv/clint.c \
       arch/riscv/plic.c \
       arch/riscv/wait.S \
       arch/riscv/syscon.c \
       arch/riscv/sbi.c \
       arch/riscv/smp.S \

CFLAGS+= -mcmodel=medany \

endif

ifeq ($(ARCH), mipsel)

LDFILE = arch/mips/linker32.ld

SRC += arch/mips/mips.c \
       arch/mips/mem32.c \
       arch/mips/irq.c \
       arch/mips/boot32.S \
       arch/mips/fpu.S \
       arch/mips/wait.S \

CFLAGS+= -mno-abicalls

endif

ifeq ($(ARCH), mips64el)

LDFILE = arch/mips/linker64.ld

SRC += arch/mips/mips.c \
       arch/mips/mem64.c \
       arch/mips.irq.c \
       arch/mips/boot64.S \
       arch/mips/fpu.S \
       arch/mips/wait.S \

CFLAGS+= -mno-abicalls

endif

ifeq ($(WITH_ACPI), yes)
SRC += kern/acpi.c \
       kern/aml.c
endif

ifeq ($(WITH_FDT), yes)
SRC += kern/fdt.c
endif

ifeq ($(WITH_LTO), yes)
CFLAGS+= -flto=auto
endif

SRC_PATH = .

OBJ_PATH = $(OBJDIR)/sys

CFLAGS+= -iquote . \
         -isystem include \
         -isystem $(OBJ_PATH)/include \
         -fno-pie \
         -fno-PIE \
         -fno-pic \
         -fno-PIC \
         -fvisibility=default \

LDFLAGS+= -no-pie \
          -rdynamic \

ifeq ($(WITH_ASLR), yes)
CPPFLAGS+= -DWITH_ASLR
endif

CPPFLAGS+= -DKSTACK_SIZE="($(KSTACK_SIZE))"

all: $(BIN_LN)

include $(MAKEDIR)/src.mk

$(OBJ_PATH)/%.tar: ../build/%.tar
	@$(LD) $(LDMODE) -r -b binary -o $@ $<

$(OBJ_PATH)/libdummy_dynamic.so:
	@$(CC) /dev/null -shared -nostdlib -ffreestanding -o $@

$(BIN): $(OBJ_FILES) $(LDFILE) $(OBJ_PATH)/usr.tar $(OBJ_PATH)/etc.tar $(OBJ_PATH)/libdummy_dynamic.so
	@$(MKDIR) -p $(dir $@)
	@$(ECHO) "LD $@"
	@$(CC) $(LDFLAGS) -T $(LDFILE) -o $@ $(OBJ_FILES) $(OBJ_PATH)/usr.tar \
	       $(OBJ_PATH)/etc.tar -lgcc -L $(OBJ_PATH) -ldummy_dynamic

$(BIN_LN): $(BIN)
	@$(LN) -sf $< $@

clean:
	@$(RM) -fr $(OBJ_FILES)
	@$(RM) -f $(BIN)

.PHONY: all clean
