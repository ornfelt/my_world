ifeq ($(ARCH), i386)

QEMU = qemu-system-i386
QEMU_ARGS = -machine q35 \
            -cpu max

ifeq ($(WITH_QEMU_KVM), yes)
QEMU_ARGS+= -enable-kvm
endif

endif

ifeq ($(ARCH), amd64)

QEMU = qemu-system-x86_64
QEMU_ARGS = -machine q35 \
            -cpu max

ifeq ($(WITH_QEMU_KVM), yes)
QEMU_ARGS+= -enable-kvm
endif

endif

ifeq ($(ARCH), aarch64)

QEMU = qemu-system-aarch64
QEMU_ARGS = -machine virt \
            -cpu cortex-a72

endif

ifeq ($(ARCH), arm)

QEMU = qemu-system-arm
QEMU_ARGS = -machine virt \
            -cpu cortex-a15

endif

ifeq ($(ARCH), riscv32)

QEMU = qemu-system-riscv32
QEMU_ARGS = -machine virt \
            -cpu rv32

endif

ifeq ($(ARCH), riscv64)

QEMU = qemu-system-riscv64
QEMU_ARGS = -machine virt \
            -cpu rv64

endif

ifeq ($(ARCH), mipsel)

QEMU = qemu-system-mipsel
QEMU_ARGS = -machine malta \
            -cpu 24Kc \

endif

ifeq ($(ARCH), mips64el)

QEMU = qemu-system-mips64el
QEMU_ARGS = -machine malta \
            -cpu MIPS64R2-generic \

endif

QEMU_ARGS+= -m $(WITH_QEMU_RAM) \
            -smp cores=$(WITH_QEMU_CPU) \

ifneq ($(filter $(ARCH), i386 amd64),)

QEMU_ARGS+= -device piix4-ide,id=ide \
            -drive id=disk,file=$(DISK_FILE),format=qcow2,if=none \
            -device ide-hd,drive=disk,bus=ide.0 \

endif

ifeq ($(ARCH), arm)

arm_flash0.img:
	dd if=/dev/zero of=$@ bs=1M count=64
	dd if=/usr/share/AAVMF/AAVMF32_CODE.fd of=$@ conv=notrunc

arm_flash1.img:
	dd if=/dev/zero of=$@ bs=1M count=64
	dd if=/usr/share/AAVMF/AAVMF32_VARS.fd of=$@ conv=notrunc
endif

ifeq ($(ARCH), aarch64)

aarch64_flash0.img:
	dd if=/dev/zero of=$@ bs=1M count=64
	dd if=/usr/share/qemu-efi-aarch64/QEMU_EFI.fd of=$@ conv=notrunc

aarch64_flash1.img:
	dd if=/dev/zero of=$@ bs=1M count=64

endif

ifneq ($(filter $(ARCH), arm aarch64),)

run: $(ARCH)_flash1.img $(ARCH)_flash0.img

QEMU_ARGS+= -drive file=$(ARCH)_flash0.img,format=raw,if=pflash \
            -drive file=$(ARCH)_flash1.img,format=raw,if=pflash \

endif

ifneq ($(filter $(ARCH), riscv32 riscv64),)

$(ARCH)_fw_jump.bin:
	@echo "$@ is missing!"
	@echo "it can be downloaded from https://github.com/riscv-software-src/opensbi/releases"
	@echo
	@false

$(ARCH)_uboot.elf:
	@echo "$@ is missing!"
	@echo "it can be compiled this way:"
	@echo "download from https://github.com/u-boot/u-boot/tags"
	@echo "make qemu-$(ARCH)_smode_defconfig"
	@echo "make CC=$(ARCH)-unknown-eklat-gcc LD=$(ARCH)-unknown-eklat-ld OBJCOPY=$(ARCH)-unknown-eklat-objcopy"
	@echo
	@false

run: $(ARCH)_fw_jump.bin $(ARCH)_uboot.elf

QEMU_ARGS+= -bios $(ARCH)_fw_jump.bin \
            -kernel $(ARCH)_uboot.elf \

endif

ifneq ($(filter $(ARCH), mipsel mips64el),)

$(ARCH)_uboot.elf:
	@echo "$@ is missing!"
	@echo "it can be compiled this way:"
	@echo "download from https://github.com/u-boot/u-boot/tags"
	@echo "make maltael_defconfig / malta64_defconfig"
	@echo "make CC=$(ARCH)-unknown-eklat-gcc LD=$(ARCH)-unknown-eklat-ld OBJCOPY=$(ARCH)-unknown-eklat-objcopy"
	@echo
	@false

$(ARCH)_flash.img: $(ARCH)_uboot.elf
	dd if=/dev/zero bs=1M count=4 | tr '\000' '\377' > $@
	dd if=$< of=$@ conv=notrunc

run: $(ARCH)_flash.img

QEMU_ARGS+= -drive file=$(ARCH)_flash.img,format=raw,if=pflash

endif

QEMU_ARGS+= -drive id=drive0,file=$(ISO_NAME),format=raw,if=none \
            -device virtio-blk,drive=drive0,bootindex=0 \

QEMU_ARGS+= -audiodev pa,id=snd0 \
            -device ich9-usb-uhci1,id=uhci \
            -device usb-ehci,id=ehci \
            -device qemu-xhci,id=xhci \
            -device usb-hub,bus=uhci.0,port=1 \

QEMU_ARGS+= -device virtio-rng-pci

ifeq ($(WITH_QEMU_PCSPEAKER), yes)
QEMU_ARGS+= -machine pcspk-audiodev=snd0
endif

ifeq ($(WITH_QEMU_AUDIO), hda)
QEMU_ARGS+= -device intel-hda -device hda-duplex,audiodev=snd0
endif

ifeq ($(WITH_QEMU_AUDIO), ac97)
QEMU_ARGS+= -device AC97,audiodev=snd0
endif

ifeq ($(WITH_QEMU_AUDIO), usb)
QEMU_ARGS+= -device usb-audio,bus=uhci.0,port=1.2,audiodev=snd0
endif

ifneq ($(WITH_QEMU_NET), none)
QEMU_ARGS+= -netdev user,id=net0,hostfwd=tcp::2222-:22
endif

ifeq ($(WITH_QEMU_NET), rtl8139)
QEMU_ARGS+= -device rtl8139,netdev=net0
endif

ifeq ($(WITH_QEMU_NET), ne2k)
QEMU_ARGS+= -device ne2k_pci,netdev=net0
endif

ifeq ($(WITH_QEMU_NET), e1000)
QEMU_ARGS+= -device e1000,netdev=net0
endif

ifeq ($(WITH_QEMU_NET), e1000e)
QEMU_ARGS+= -device e1000e,netdev=net0
endif

ifeq ($(WITH_QEMU_NET), virtio)
QEMU_ARGS+= -device virtio-net,netdev=net0
endif

ifeq ($(WITH_QEMU_KBD), usb)
QEMU_ARGS+= -device usb-kbd,bus=uhci.0,port=1.1
endif

ifeq ($(WITH_QEMU_KBD), virtio)
QEMU_ARGS+= -device virtio-keyboard-pci
endif

ifeq ($(WITH_QEMU_MOUSE), usb)
QEMU_ARGS+= -device usb-mouse,bus=uhci.0,port=2
endif

ifeq ($(WITH_QEMU_MOUSE), virtio)
QEMU_ARGS+= -device virtio-mouse-pci
endif

ifeq ($(WITH_QEMU_GRAPHICS), none)
QEMU_ARGS+= -nographic \
            -serial mon:stdio \
            -vga none
endif

ifeq ($(WITH_QEMU_GRAPHICS), vga)
QEMU_ARGS+= -vga std
endif

ifeq ($(WITH_QEMU_GRAPHICS), virtio-gpu)
QEMU_ARGS+= -device virtio-gpu \
            -vga none
endif

ifeq ($(WITH_QEMU_GRAPHICS), virtio-gpu-gl)
QEMU_ARGS+= -device virtio-gpu-gl \
            -display gtk,gl=on \
            -vga none
endif

ifneq ($(WITH_QEMU_TPM), none)
QEMU_ARGS+= -chardev socket,id=chrtpm,path=$(WITH_QEMU_TPM) \
            -tpmdev emulator,id=tpm0,chardev=chrtpm \
            -device tpm-tis,tpmdev=tpm0
endif
