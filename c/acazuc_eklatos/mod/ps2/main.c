#include "ps2.h"

#include "arch/x86/asm.h"
#include "arch/x86/x86.h"

#include <kmod.h>

int ps2_wr(uint8_t port, uint8_t v)
{
	size_t i = 0;
	while ((inb(PS2_STATUS) & 2))
	{
		if (++i == PS2_TIMEOUT)
			return 1;
		io_wait();
	}
	outb(port, v);
	return 0;
}

int ps2_rd(uint8_t port, uint8_t *v)
{
	size_t i = 0;
	while (!(inb(PS2_STATUS) & 1))
	{
		if (++i == PS2_TIMEOUT)
			return 1;
		io_wait();
	}
	*v = inb(port);
	return 0;
}

int ps2_wait_ack(void)
{
	uint8_t ack;
	if (ps2_rd(PS2_DATA, &ack))
		return 1;
	if (ack != 0xFA)
		return 1;
	return 0;
}

void ps2_interrupt(void *userptr)
{
	(void)userptr;

	if (inb(PS2_STATUS) & 0x20)
		ps2_mouse_input(inb(PS2_DATA));
	else
		ps2_kbd_input(inb(PS2_DATA));
}

static int init(void)
{
	ps2_kbd_init();
	ps2_mouse_init();
	return 0;
}

static void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "ps2",
	.init = init,
	.fini = fini,
};
