export ROOTDIR = $(PWD)
export MAKEDIR = $(ROOTDIR)/mk

BIN_NAME = sys/eklatos.bin
ISO_NAME = eklatos.iso
DISK_FILE = disk.qcow2

all: $(ISO_NAME)

include $(MAKEDIR)/env.mk
include $(MAKEDIR)/vrt.mk

sys_arch:
	@$(MKDIR) -p $(OBJDIR)/sys/include
	@$(LN) -sfn $(ROOTDIR)/sys/arch/$(ARCH_SYSDIR)/include $(OBJDIR)/sys/include/arch

stdinc:
	@$(MKDIR) -p "$(BUILDDIR)/usr/include"
	@$(CP) -Ru lib/libc/include/* "$(BUILDDIR)/usr/include"
	@$(CP) -Ru lib/libm/include/* "$(BUILDDIR)/usr/include"
	@$(CP) -Ru lib/libm/src/include/* "$(BUILDDIR)/usr/include"
	@$(CP) -Ru lib/libm/src/src/*.h "$(BUILDDIR)/usr/include"
	@$(CP) -Ru lib/libdl/include/* "$(BUILDDIR)/usr/include"
	@$(CP) -Ru lib/libutil/include/* "$(BUILDDIR)/usr/include"
	@$(CP) -Ru lib/libpthread/include/* "$(BUILDDIR)/usr/include"

cc: stdinc
	@$(MAKE) -C cc

lib: crt
	@$(MAKE) -C lib

crt: stdinc
	@$(MAKE) -C crt

bin: lib crt ext
	@$(MAKE) -C bin

etc:
	@$(MAKE) -C etc

ext: lib
	@$(MAKE) -C ext

build: bin etc mod
	@$(MAKE) -C build

mod: sys_arch
	@$(MAKE) -C mod

$(BIN_NAME): build sys_arch
	@$(MAKE) -C sys

$(ISO_NAME): $(BIN_NAME) grub.cfg
	@$(MKDIR) -p $(dir $@)
	@$(MKDIR) -p iso/boot
	@$(CP) $(BIN_NAME) iso/boot
	@$(MKDIR) -p iso/boot/grub
	@$(CP) grub.cfg iso/boot/grub
	@$(ROOTDIR)/cc/$(ARCH)/bin/$(HOST)-grub-mkrescue -o "$@" iso -- -hfsplus off

$(DISK_FILE):
	@$(ECHO) "creating 100M disk image"
	@mkdir -p disk
	@sudo virt-make-fs --format=qcow2 --partition=mbr --type=ext2 --size=128M disk $(DISK_FILE)
	@sudo chown $(USER) $(DISK_FILE)

run: $(ISO_NAME) $(DISK_FILE)
	@$(QEMU) $(QEMU_ARGS)

doc-html:
	@$(MAKE) -C doc html

size:
	@wc `find lib bin sys mod crt -type f \( -name \*.\[chS\] -a \! -path lib/libm/\* \)` | tail -n 1

checksec:
	@find $(BUILDDIR)/usr/bin $(BUILDDIR)/usr/lib -type f -executable -exec checksec --file={} \;

clean:
	@$(RM) -f $(ISO_NAME)
	@$(MAKE) -C etc clean
	@$(MAKE) -C crt clean
	@$(MAKE) -C lib clean
	@$(MAKE) -C ext clean
	@$(MAKE) -C bin clean
	@$(MAKE) -C mod clean
	@$(MAKE) -C sys clean

cleanall: clean
	@$(RM) -fr $(BUILDDIR)

cleancc:
	@$(MAKE) -C cc clean

.PHONY: all clean run bin lib sys crt etc ext build stdinc doc-html mod cc cleanall checksec sys_arch
