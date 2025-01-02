#define ENABLE_TRACE

#include "evdev.h"
#include "ps2.h"

#include "arch/x86/x86.h"

#include <std.h>

#define PS2_MOUSE_RESET           0xFF
#define PS2_MOUSE_RESEND          0xFE
#define PS2_MOUSE_DEFAULT         0xF6
#define PS2_MOUSE_STREAM_OFF      0xF5
#define PS2_MOUSE_STREAM_ON       0xF4
#define PS2_MOUSE_SET_SAMPLE_RATE 0xF3
#define PS2_MOUSE_GET_MOUSE_ID    0xF2
#define PS2_MOUSE_SET_REMOTE_MODE 0xF0
#define PS2_MOUSE_SET_WRAP_MODE   0xEE
#define PS2_MOUSE_RESET_WRAP_MODE 0xEC
#define PS2_MOUSE_REQUEST_PACKET  0xEB
#define PS2_MOUSE_SET_STREAM_MODE 0xEA
#define PS2_MOUSE_STATUS_REQUEST  0xE9
#define PS2_MOUSE_SET_RESOLUTION  0xE8
#define PS2_MOUSE_SET_SCALE_2_1   0xE7
#define PS2_MOUSE_SET_SCALE_1_1   0xE6

static uint8_t buf[4];
static size_t buf_pos;
static uint8_t mouseid;
static uint8_t buttons_state;
static struct irq_handle mouse_irq_handle;
static struct evdev *mouse_evdev;

static int ps2_mouse_wr(uint8_t cmd)
{
	return ps2_wr(PS2_CMD, PS2_CMD_SEND_P2_IN)
	    || ps2_wr(PS2_DATA, cmd);
}

void ps2_mouse_input(uint8_t value)
{
#if 0
	TRACE("ps2 mouse 0x%02x", value);
#endif

	buf[buf_pos++] = value;
	if (mouseid)
	{
		if (buf_pos != 4)
			return;
		buf_pos = 0;
	}
	else
	{
		if (buf_pos != 3)
			return;
		buf_pos = 0;
	}
	if (mouseid)
	{
		switch (buf[3] & 0xF)
		{
			case 0x1:
				ev_send_scroll_event(mouse_evdev, 0, 1);
				break;
			case 0xF:
				ev_send_scroll_event(mouse_evdev, 0, -1);
				break;
			case 0x2:
				ev_send_scroll_event(mouse_evdev, 1, 0);
				break;
			case 0xE:
				ev_send_scroll_event(mouse_evdev, -1, 0);
				break;
			default:
				break;
		}
	}
	int16_t x = 0;
	if (!(buf[0] & (1 << 6)))
	{
		if (buf[0] & (1 << 4))
			x = buf[1] - 0x100;
		else
			x = buf[1];
	}
	int16_t y = 0;
	if (!(buf[0] & (1 << 7)))
	{
		if (buf[0] & (1 << 5))
			y = buf[2] - 0x100;
		else
			y = buf[2];
	}
	if (x || y)
		ev_send_pointer_event(mouse_evdev, x, -y);
	uint8_t buttons = buf[0] & 0x7;
	if (mouseid == 4)
		buttons |= (buf[3] >> 1) & 0x18;
	uint8_t buttons_delta = buttons ^ buttons_state;
	buttons_state = buttons;

#define BUTTON_TEST(buttons, delta, flag, button) \
do \
{ \
	if ((delta) & (flag)) \
	{ \
		if ((buttons) & (flag)) \
			ev_send_mouse_event(mouse_evdev, button, 1); \
		else \
			ev_send_mouse_event(mouse_evdev, button, 0); \
	} \
} while (0)

	BUTTON_TEST(buttons, buttons_delta, 1 << 0, MOUSE_BUTTON_LEFT);
	BUTTON_TEST(buttons, buttons_delta, 1 << 1, MOUSE_BUTTON_RIGHT);
	BUTTON_TEST(buttons, buttons_delta, 1 << 2, MOUSE_BUTTON_MIDDLE);
	BUTTON_TEST(buttons, buttons_delta, 1 << 3, MOUSE_BUTTON_4);
	BUTTON_TEST(buttons, buttons_delta, 1 << 4, MOUSE_BUTTON_5);

#undef BUTTON_TEST
}

void ps2_mouse_init(void)
{
	uint8_t tmp;

	if (ps2_wr(PS2_CMD, PS2_CMD_TEST_P2)
	 || ps2_rd(PS2_DATA, &tmp)
	 || tmp != 0x00)
		goto err;

	if (ps2_wr(PS2_CMD, PS2_CMD_READ_STATUS)
	 || ps2_rd(PS2_DATA, &tmp))
		goto err;

	if (ps2_wr(PS2_CMD, PS2_CMD_WRITE_STATUS)
	 || ps2_wr(PS2_DATA, tmp | 0x2))
		goto err;

	if (ps2_wr(PS2_CMD, PS2_CMD_ENABLE_P2))
		goto err;

#if 0
	if (ps2_mouse_wr(PS2_MOUSE_RESET))
		goto err;

	size_t i = 0;
	while (1)
	{
		if (++i == PS2_TIMEOUT)
			goto err;
		if (ps2_rd(PS2_DATA, &tmp))
			goto err;
		if (tmp == 0xAA)
			break;
	}
	if (ps2_rd(PS2_DATA, &tmp)) /* 0x00 */
		goto err;
#endif

	if (ps2_mouse_wr(PS2_MOUSE_DEFAULT)
	 || ps2_wait_ack())
		goto err;

	if (ps2_mouse_wr(PS2_MOUSE_GET_MOUSE_ID)
	 || ps2_wait_ack()
	 || ps2_rd(PS2_DATA, &mouseid))
		goto err;

	if (!mouseid)
	{
		if (ps2_mouse_wr(PS2_MOUSE_SET_SAMPLE_RATE)
		 || ps2_mouse_wr(200)
		 || ps2_rd(PS2_DATA, &tmp) /* 0xFE */
		 || ps2_wait_ack()
		 || ps2_mouse_wr(PS2_MOUSE_SET_SAMPLE_RATE)
		 || ps2_mouse_wr(100)
		 || ps2_rd(PS2_DATA, &tmp) /* 0xFE */
		 || ps2_wait_ack()
		 || ps2_mouse_wr(PS2_MOUSE_SET_SAMPLE_RATE)
		 || ps2_mouse_wr(80)
		 || ps2_rd(PS2_DATA, &tmp) /* 0xFE */
		 || ps2_wait_ack()
		 || ps2_mouse_wr(PS2_MOUSE_GET_MOUSE_ID)
		 || ps2_wait_ack()
		 || ps2_rd(PS2_DATA, &tmp))
			goto err;
		if (tmp == 3)
		{
			mouseid = 3;
			if (ps2_mouse_wr(PS2_MOUSE_SET_SAMPLE_RATE)
			 || ps2_mouse_wr(200)
			 || ps2_rd(PS2_DATA, &tmp) /* 0xFE */
			 || ps2_wait_ack()
			 || ps2_mouse_wr(PS2_MOUSE_SET_SAMPLE_RATE)
			 || ps2_mouse_wr(200)
			 || ps2_rd(PS2_DATA, &tmp) /* 0xFE */
			 || ps2_wait_ack()
			 || ps2_mouse_wr(PS2_MOUSE_SET_SAMPLE_RATE)
			 || ps2_mouse_wr(80)
			 || ps2_rd(PS2_DATA, &tmp) /* 0xFE */
			 || ps2_wait_ack()
			 || ps2_mouse_wr(PS2_MOUSE_GET_MOUSE_ID)
			 || ps2_wait_ack()
			 || ps2_rd(PS2_DATA, &tmp))
				goto err;
			if (tmp == 4)
				mouseid = 4;
		}
	}

	if (ps2_mouse_wr(PS2_MOUSE_STREAM_ON)
	 || ps2_wait_ack())
		goto err;

	if (evdev_alloc(&mouse_evdev))
	{
		TRACE("ps2: failed to alloc mouse evdev");
		return;
	}

	if (register_isa_irq(ISA_IRQ_MOUSE, ps2_interrupt, NULL, &mouse_irq_handle))
	{
		TRACE("ps2: failed to enable mouse IRQ\n");
		return;
	}

	return;

err:
	TRACE("ps2: mouse not found");
}
