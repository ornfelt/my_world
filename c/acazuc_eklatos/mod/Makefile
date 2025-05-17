include $(MAKEDIR)/env.mk

export KERNEL = yes

DIRS = ac97 \
       evdev \
       fs/ext2 \
       fs/fat \
       fs/iso9660 \
       hda \
       net/tcp \
       net/udp \
       nic/em \
       nic/ne2k \
       nic/rtl8139 \
       nic/rtl8169 \
       pci \
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
