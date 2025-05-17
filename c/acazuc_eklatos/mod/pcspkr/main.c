#include "arch/x86/asm.h"

#include <kmod.h>
#include <file.h>
#include <time.h>
#include <vfs.h>
#include <std.h>
#include <uio.h>

struct cdev *dev_pcspkr;

void
set_freq(uint32_t freq)
{
	if (!freq)
	{
		outb(0x61, inb(0x61) & 0xFC);
		return;
	}
	uint32_t div = 1193180 / freq;
	outb(0x43, 0xB6);
	outb(0x42, div);
	outb(0x42, div >> 8);
	uint8_t tmp = inb(0x61);
	if (tmp != (tmp | 3))
		outb(0x61, tmp | 3);
}

ssize_t
pcspkr_write(struct file *file, struct uio *uio)
{
	(void)file;
	uint16_t freq;
	ssize_t ret = uio_copyout(&freq, uio, sizeof(uio));
	if (ret < 0)
		return ret;
	if (ret < 2)
		return ret;
	set_freq(freq);
	return ret;
}

const struct file_op
fop =
{
	.write = pcspkr_write,
};

int
init(void)
{
	int ret = cdev_alloc("pcspkr", 0, 0, 0600, makedev(10, 0), &fop, &dev_pcspkr);
	if (ret)
		printf("failed to create /dev/pcspkr: %s\n", strerror(ret));
	return ret;
}

void
fini(void)
{
}

struct kmod_info
kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "pcspkr",
	.init = init,
	.fini = fini,
};
