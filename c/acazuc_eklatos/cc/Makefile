include $(MAKEDIR)/env.mk

export PREFIX = $(ROOTDIR)/cc/build
export SYSROOT = $(ROOTDIR)/build/$(ARCH)

all: grub gdb

grub: gcc
	@$(MAKE) -C grub

gcc: binutils
	@$(MAKE) -C gcc

binutils:
	@$(MAKE) -C binutils

gdb: gcc
	@$(MAKE) -C gdb

clean:
	@$(MAKE) -C gdb clean
	@$(MAKE) -C grub clean
	@$(MAKE) -C gcc clean
	@$(MAKE) -C binutils clean

.PHONY: gcc binutils grub gdb clean
