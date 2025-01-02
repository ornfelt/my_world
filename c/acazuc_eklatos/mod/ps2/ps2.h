#ifndef PS2_H
#define PS2_H

#include <std.h>

#define PS2_STATUS 0x64
#define PS2_CMD    0x64
#define PS2_DATA   0x60

#define PS2_TIMEOUT 0x1000

#define PS2_CMD_READ_STATUS  0x20
#define PS2_CMD_WRITE_STATUS 0x60
#define PS2_CMD_DISABLE_P2   0xA7
#define PS2_CMD_ENABLE_P2    0xA8
#define PS2_CMD_TEST_P2      0xA9
#define PS2_CMD_TEST_CTL     0xAA
#define PS2_CMD_TEST_P1      0xAB
#define PS2_CMD_DISABLE_P1   0xAD
#define PS2_CMD_ENABLE_P1    0xAE
#define PS2_CMD_SEND_P1_OUT  0xD2
#define PS2_CMD_SEND_P2_OUT  0xD3
#define PS2_CMD_SEND_P2_IN   0xD4

void ps2_kbd_init(void);
void ps2_mouse_init(void);
int ps2_wait_ack(void);

void ps2_kbd_input(uint8_t value);
void ps2_mouse_input(uint8_t value);
void ps2_interrupt(void *userptr);

int ps2_wr(uint8_t port, uint8_t v);
int ps2_rd(uint8_t port, uint8_t *v);

#endif
