include $(MAKEDIR)/env.mk

export KERNEL = yes

DIRS = ac97 \
       em \
       evdev \
       ext2fs \
       fatfs \
       hda \
       iso9660 \
       ne2k \
       net/tcp \
       net/udp \
       pci \
       rtl8139 \
       rtl8169 \
       snd \
       usb/audio \
       usb/core \
       usb/ehci \
       usb/hub \
       usb/kbd \
       usb/mouse \
       usb/uhci \
       virtio/blk \
       virtio/core \
       virtio/gpu \
       virtio/input \
       virtio/net \
       virtio/rng \

ifneq ($(filter $(ARCH), i386 amd64),)
DIRS+= ata \
       pcspkr \
       ps2
endif

include $(MAKEDIR)/dir.mk
