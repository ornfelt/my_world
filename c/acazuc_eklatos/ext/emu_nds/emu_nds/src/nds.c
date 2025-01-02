#include "nds.h"
#include "mbc.h"
#include "mem.h"
#include "apu.h"
#include "cpu.h"
#include "gpu.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INACCURACY_SHIFT 3 /* the power of two of the "batch" size for cycles interpretation */

/*
 * 1130: bios call wrapper of 20BC (by 1164)
 * 1164: bios safe call
 * 20BC: decrypt a block of 8 bytes (r0 = dst, r1 = src)
 * 22A4: read a firmware byte
 * 227C: read an spi byte (return in r0, r0 = SPICNT (bit 15 not needed), r1 = hold)
 * 22D6: read spi bytes (r1 = dst, r2 = bytes count, r3 = unk (unused?))
 * 2330: function to read a block byte per byte (with cached data)
 *  - read + decrypt a firmware block into 0x037F90C4 (8 bytes, 0x037F800C as tmp buffer)
 *  - increment uint16_t @ 0x037F800A (if not 0 at call, no data is read + unciphered) (upper 29 bits are cleared (mod 8))
 *  - return 0x037F800C[*0x037F800A] in r0
 * 2368: wait SPI availability & write SPIDATA byte (r0=spicnt ored mask, r1=spidata byte, r2=chip hold(0/1))
 *       237C: while (SPICNT & (1 << 7))
 *       2382: SPICNT = val
 *       2384: write SPIDATA
 * 2388: (called 1 time) do things, then call 2330 4 times
 *       2398: 33C0(r0=3, r1=1)
 * 2462: calls LZ77UnCompReadByCallbackWrite16bit with:
 *             - open_and_get_32bit: 0x2388 (returns 0x010b4410, so LZ44 of 0x10b44 uncompressed bytes)
 *             - close             : 0x22C6
 *             - get_8bit          : 0x2330
 *             - get_16bit / get_32bit as null
 *
 *             read firmware up to CA18
 *       24A8: if (SVC_LZ77UnCompReadByCallbackWrite16bit(r0=0x0200, r1=0x02320000, r2=0x33E0, r3=0x33E0) > 0)
 *       24B6      SVC_GetCRC16(r0=0xFFFF, r1=0x02320000, r2=0x10B44) = 0x245F
 *
 *             read firmware up to 14F18
 *       24F2: if (SVC_LZ77UnCompReadBycallbackWrite16bit(r0=0xCA20, r1=0x037FA800, r2=0x33E0, r3=0x33E0) > 0)
 *       2500:     SVC_GetCRC16(r0=0x245F, r1=0x037FA800, r2=0x0B2B0) = 0x0F1F
 * 27F8: decrypt secure area
 *       2850: call decrypt on first 8 bytes
 *       285A: cmp first 4 bytes of "encryObj"
 *       2862: cmp last 4 bytes of "encryObj"
 *       286E: while (remaining > 0) decrypt 8 bytes
 *       289E: CpuFastSet(r0=0x037f90c4, r1=0x02000000, r2=0x200): copy the decrypted to 0x2000000
 * 2A2A: LZ77UnCompReadByCallbackWrite16bit
 * 3344: thumb wrapper of 1130
 * 33A4: read spi bytes + WaitByLoop (r0 = dst, r1 = bytes count, r2 = unk (unused?))
 * 33C0: firmware write byte ? (r0=spidata byte, r1=chip hold)
 *       33CC: 2368(r0=0x2100, r1=arg_r0, r2=arg_r1)
 *       33D2: if (!arg_r1) WaitByLoop(3)
 *
 *
 * - 124A
 *   - 2462 @ 1313
 *     - 2A2A
 *       - 2388
 *         - *
 *         - 2330
 *           - 33A4
 *             - 22D6
 *               - 227C
 *           - 20BC (indirect)
 *
 * 13BC: IPCSYNC = 0
 * 13C6: IPCSYNC = 1
 * 1424: IPCSYNC = 2
 * 144C: IPCSYNC = 3 (unstuck ARM9 @ 0x3E4)
 *
 *
 * 13B6:
 *       stuff
 *       calls 137A
 *       calls 245A
 * 245A: 2436(r0=0, r1=0x027FF830, r2=0x20)
 * 2436: 2388(r0=passthrough, r1=0, r2=0)
 *       33A4(r0=arg_r1, r1=arg_r2, r2=1)
 * 2388: stuff
 * 137A: store 1910 result in *0x027FF800
 * 1910: call 1888 (return chipid in r0)
 * dst = 0x0380fecc
 * struct cmd
 * {
 *     uint32_t result;
 *     uint32_t ROMCTRL_MASK (to be ored with 0xA7000000);
 *     uint32_t unk2;
 *     uint64_t cmd;
 * };
 * 1888: get ROMID ? (r0 = struct cmd*)
 *       1890: struct->result = -1;
 *       1896: call 1698 (with r0 = dst)
 *       18A2: start ROMCTRL transfer
 *       18A4: wait for ROMCTRL ready
 *       18AE: load result
 * 1698: (r0 = struct cmd*)
 *       16A0: call 166A
 *       16A8: enable AUXSPICNT IRQ / slot enable
 *       16B2: loop two times:
 *             r1 = dst[(i - 1) * 4]
 *             ROM_CMDOUT[i * 4] = r1
 * 166A:
 *       166C: wait for ROMCTRL ready
 * 1DC4: (irq vector) do stuff
 *       call 1888
 *
 * 21E8: read RTC date
 *
 * 214A: write RTC cmd (r0=rtc reg write mask)
 *
 * 20F8: read RTC bytes (r0=dst, r1=nbytes)
 *       2104: while (i < nbytes)
 *       2108: while (b < 8)
 *             211C: read RTC bit
 *
 * 038033e8: touchscreen write byte (always 0x84)
 * 038033f8: check for !busy bit
 * 03803404: write SPI byte 0
 * 03803414: check for !busy bit
 * 0380341e: SPICNT = 0x8210
 * 03803424: write SPI byte 0
 * 03803434: check for !busy bit
 * 03803442: return to 03802fa0
 * 03802fa0: call 03803c24
 * 03803c24: some WaitByLoop()-like fn with 0x10 iterations
 * 03803c34: return to 03802fa4
 *
 * unk:
 *           02327c28: 02327b98(r0=01da6a00, r1=023556e0, r2=00000200)

 * 02327b98: function (r0=cartridge addr, r1=data destination, r2=bytes)
 *           02327bb0: 02327b60(r0=0x03002ef4)
 *           02327bbc: start transfer of cartridge icon / name read transfer (DMA to 0x002002200 - 0x02002400)
 *           02327bc6: if (ROMCTRL & (1 << 23)) goto 02327bde
 *           02327bd0: read word from ROMDATA
 *           02327bde: while (ROMCTRL & (1 << 31)) goto 02327bc6
 *
 * 02327b60: function (r0=cmd ptr)
 *           02327b66: AUXSPICNT[1] = 0x80
 *           02327b76: ROMCMD[0] = cmd[0x4]
 *           02327b7a: ROMCMD[1] = cmd[0x5]
 *           02327b7e: ROMCMD[2] = cmd[0x6]
 *           02327b80: ROMCMD[3] = cmd[0x7]
 *           02327b76: ROMCMD[0] = cmd[0x0]
 *           02327b7a: ROMCMD[1] = cmd[0x1]
 *           02327b7e: ROMCMD[2] = cmd[0x2]
 *           02327b80: ROMCMD[3] = cmd[0x3]
 *
 * 02002000: 0x01da6a00 (0x200 bytes)
 * 02002200: 0x0030c800 (0xA00 bytes) (all the icon + names) (seems to be "ASLR" in 4*0x1000 box, gets overwritten by next copy)
 * 02004000: 0x00008000 (0x2E4800 bytes)
 *
 * arm9 rom offset:   0x4000
 * arm9 entry:        0x02000800
 * arm9 ram address:  0x02000000
 * arm9 size:         0xEE238
 * arm7 rom offset:   0x2E3800
 * arm7 entry:        0x02380000
 * arm7 ram address:  0x02380000
 * arm7 size:         0x2869C
 * icon/title offset: 0x30C800
 */

/* this multithreading looks stupid (and it is)
 * but it gives about 15% more fps on firmware titlescreen
 * it doesn't have any drawbacks (maybe some race conditions
 * if cpu changes vram mapping but nothing more)
 *
 * forgive me scheduler gods though...
 */
#ifdef ENABLE_MULTITHREAD

static void *gpu_loop(void *arg)
{
	nds_t *nds = arg;
	while (1)
	{
		pthread_mutex_lock(&nds->gpu_mutex);
		pthread_cond_wait(&nds->gpu_cond, &nds->gpu_mutex);
		__atomic_store_n(&nds->gpu_y, 0, __ATOMIC_SEQ_CST);
		pthread_mutex_unlock(&nds->gpu_mutex);
		for (uint8_t y = 0; y < 192; ++y)
		{
			gpu_draw(nds->gpu, y);
			__atomic_store_n(&nds->gpu_y, y + 1, __ATOMIC_SEQ_CST);
			while (__atomic_load_n(&nds->nds_y, __ATOMIC_SEQ_CST) < nds->gpu_y)
				;
		}
		while (!__atomic_load_n(&nds->nds_g3d, __ATOMIC_SEQ_CST))
			;
		gpu_g3d_draw(nds->gpu);
		__atomic_store_n(&nds->gpu_g3d, 1, __ATOMIC_SEQ_CST);
	}
	return NULL;
}

#endif

struct nds *nds_new(const void *rom_data, size_t rom_size)
{
	struct nds *nds = calloc(sizeof(*nds), 1);
	if (!nds)
		return NULL;

	nds->mbc = mbc_new(nds, rom_data, rom_size);
	if (!nds->mbc)
		return NULL;

	nds->mem = mem_new(nds, nds->mbc);
	if (!nds->mem)
		return NULL;

	nds->apu = apu_new(nds->mem);
	if (!nds->apu)
		return NULL;

	nds->arm7 = cpu_new(nds->mem, 0);
	if (!nds->arm7)
		return NULL;

	nds->arm9 = cpu_new(nds->mem, 1);
	if (!nds->arm9)
		return NULL;

	nds->gpu = gpu_new(nds->mem);
	if (!nds->gpu)
		return NULL;

#ifdef ENABLE_MULTITHREAD
	if (pthread_cond_init(&nds->gpu_cond, NULL)
	 || pthread_mutex_init(&nds->gpu_mutex, NULL)
	 || pthread_create(&nds->gpu_thread, NULL, gpu_loop, nds))
		return NULL;
#endif
	return nds;
}

void nds_del(struct nds *nds)
{
	if (!nds)
		return;
	mbc_del(nds->mbc);
	mem_del(nds->mem);
	apu_del(nds->apu);
	cpu_del(nds->arm7);
	cpu_del(nds->arm9);
	gpu_del(nds->gpu);
	free(nds);
}

static void nds_cycles(struct nds *nds, uint32_t cycles)
{
	for (; cycles; cycles -= 4)
	{
		nds->cycle += 4;
		if (!(nds->cycle & ((0x8 << INACCURACY_SHIFT) - 1)))
		{
			mem_dma(nds->mem, 0x1 << INACCURACY_SHIFT);
			mem_timers(nds->mem, 0x4 << INACCURACY_SHIFT);
			apu_cycles(nds->apu, 0x2 << INACCURACY_SHIFT);
			apu_sample(nds->apu, 0x8 << INACCURACY_SHIFT);
		}
		if (!nds->arm7->irq_wait)
		{
			if (nds->arm7->instr_delay <= 0)
			{
				cpu_cycle(nds->arm7);
				cpu_cycle(nds->arm7);
			}
			else
			{
				nds->arm7->instr_delay -= 2;
			}
		}
		if (!nds->arm9->irq_wait)
		{
			if (nds->arm9->instr_delay <= 0)
			{
				cpu_cycle(nds->arm9);
				cpu_cycle(nds->arm9);
				cpu_cycle(nds->arm9);
				cpu_cycle(nds->arm9);
			}
			else
			{
				nds->arm9->instr_delay -= 4;
			}
		}
	}
}

void nds_frame(struct nds *nds, uint8_t *video_top_buf, uint32_t video_top_pitch,
               uint8_t *video_bot_buf, uint32_t video_bot_pitch, int16_t *audio_buf,
               uint32_t joypad, uint8_t touch_x, uint8_t touch_y, uint8_t touch)
{
#if 0
	printf("touch: %d @ %dx%d\n", touch, touch_x, touch_y);
#endif
	nds->gpu->capture = mem_arm9_get_reg32(nds->mem, MEM_ARM9_REG_DISPCAPCNT) & (1 << 31);
	uint32_t powcnt1 = mem_arm9_get_reg32(nds->mem, MEM_ARM9_REG_POWCNT1);
	if (powcnt1 & (1 << 15))
	{
		nds->gpu->enga.pitch = video_top_pitch;
		nds->gpu->engb.pitch = video_bot_pitch;
		nds->gpu->enga.data = video_top_buf;
		nds->gpu->engb.data = video_bot_buf;
	}
	else
	{
		nds->gpu->enga.pitch = video_bot_pitch;
		nds->gpu->engb.pitch = video_top_pitch;
		nds->gpu->enga.data = video_bot_buf;
		nds->gpu->engb.data = video_top_buf;
	}
	nds->apu->data = audio_buf;
	nds->apu->sample = 0;
	nds->apu->clock = 0;
	nds->apu->next_sample = nds->apu->clock;
	nds->joypad = joypad;
	nds->touch = touch;
	nds->touch_x = touch_x;
	nds->touch_y = touch_y;
	gpu_commit_bgpos(nds->gpu);
#ifdef ENABLE_MULTITHREAD
	pthread_mutex_lock(&nds->gpu_mutex);
	__atomic_store_n(&nds->nds_g3d, 0, __ATOMIC_SEQ_CST);
	__atomic_store_n(&nds->nds_y, 0, __ATOMIC_SEQ_CST);
	pthread_cond_signal(&nds->gpu_cond);
	pthread_mutex_unlock(&nds->gpu_mutex);
#else
	gpu_g3d_draw(nds->gpu);
#endif
	for (uint8_t y = 0; y < 192; ++y)
	{
		mem_arm9_set_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT, (mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) & 0xFFFC) | 0x0);
		mem_arm7_set_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT, (mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) & 0xFFFC) | 0x0);
		mem_arm9_set_reg16(nds->mem, MEM_ARM9_REG_VCOUNT, y);

		if ((mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) & (1 << 5))
		 && y == (((mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) >> 8) & 0xFF)
		        | ((mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) << 1) & 0x100)))
			mem_arm9_irq(nds->mem, 1 << 2);
		if ((mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) & (1 << 5))
		 && y == (((mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) >> 8) & 0xFF)
		        | ((mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) << 1) & 0x100)))
			mem_arm7_irq(nds->mem, 1 << 2);

		nds_cycles(nds, 256 * 12);
#ifdef ENABLE_MULTITHREAD
		while (__atomic_load_n(&nds->gpu_y, __ATOMIC_SEQ_CST) < y + 1)
			;
#else
		/* draw */
		gpu_draw(nds->gpu, y);
#endif

		/* hblank */
		mem_arm9_set_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT, (mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) & 0xFFFC) | 0x2);
		mem_arm7_set_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT, (mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) & 0xFFFC) | 0x2);
		if (mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) & (1 << 4))
			mem_arm9_irq(nds->mem, 1 << 1);
		if (mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) & (1 << 4))
			mem_arm7_irq(nds->mem, 1 << 1);
		mem_hblank(nds->mem);

		nds_cycles(nds, 99 * 12);
#ifdef ENABLE_MULTITHREAD
		__atomic_store_n(&nds->nds_y, y + 1, __ATOMIC_SEQ_CST);
#endif
	}

	mem_arm9_set_reg32(nds->mem, MEM_ARM9_REG_DISPCAPCNT,
	                   mem_arm9_get_reg32(nds->mem, MEM_ARM9_REG_DISPCAPCNT) & ~(1 << 31));
	gpu_g3d_swap_buffers(nds->gpu);
	if (mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) & (1 << 3))
		mem_arm9_irq(nds->mem, 1 << 0);
	if (mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) & (1 << 3))
		mem_arm7_irq(nds->mem, 1 << 0);
	mem_vblank(nds->mem);

	for (uint16_t y = 192; y < 263; ++y)
	{
		mem_arm9_set_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT, (mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) & 0xFFFC) | 0x1);
		mem_arm7_set_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT, (mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) & 0xFFFC) | 0x1);
		mem_arm9_set_reg16(nds->mem, MEM_ARM9_REG_VCOUNT, y);

		if ((mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) & (1 << 5))
		 && y == (((mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) >> 8) & 0xFF)
		        | ((mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) << 1) & 0x100)))
			mem_arm9_irq(nds->mem, 1 << 2);
		if ((mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) & (1 << 5))
		 && y == (((mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) >> 8) & 0xFF)
		        | ((mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) << 1) & 0x100)))
			mem_arm7_irq(nds->mem, 1 << 2);

		/* vblank */
		nds_cycles(nds, 256 * 12);

		/* hblank */
		mem_arm9_set_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT, (mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) & 0xFFFC) | 0x3);
		mem_arm7_set_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT, (mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) & 0xFFFC) | 0x3);
		if (mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_DISPSTAT) & (1 << 4))
			mem_arm9_irq(nds->mem, 1 << 1);
		if (mem_arm7_get_reg16(nds->mem, MEM_ARM7_REG_DISPSTAT) & (1 << 4))
			mem_arm7_irq(nds->mem, 1 << 1);

		nds_cycles(nds, 99 * 12);
#ifdef ENABLE_MULTITHREAD
		if (y == 216)
		{
			__atomic_store_n(&nds->gpu_g3d, 0, __ATOMIC_SEQ_CST);
			__atomic_store_n(&nds->nds_g3d, 1, __ATOMIC_SEQ_CST);
		}
#endif
	}

#ifdef ENABLE_MULTITHREAD
	while (!__atomic_load_n(&nds->gpu_g3d, __ATOMIC_SEQ_CST))
		;
#endif
}

void nds_set_arm7_bios(struct nds *nds, const uint8_t *data)
{
	memcpy(nds->mem->arm7_bios, data, 0x4000);
}

void nds_set_arm9_bios(struct nds *nds, const uint8_t *data)
{
	memcpy(nds->mem->arm9_bios, data, 0x1000);
}

void nds_set_firmware(struct nds *nds, const uint8_t *data)
{
	memcpy(nds->mem->firmware, data, 0x40000);
	memcpy(nds->mem->sram, data, 0x40000);
}

void nds_get_mbc_ram(struct nds *nds, uint8_t **data, size_t *size)
{
	*data = nds->mem->sram;
	*size = nds->mem->sram_size;
}

void nds_get_mbc_rtc(struct nds *nds, uint8_t **data, size_t *size)
{
	*size = sizeof(nds->mem->rtc.offset);
	*data = (uint8_t*)&nds->mem->rtc.offset;
}

void nds_test_keypad_int(struct nds *nds)
{
	uint16_t keycnt = mem_arm9_get_reg16(nds->mem, MEM_ARM9_REG_KEYCNT);
	if (!(keycnt & (1 << 14)))
		return;
	uint16_t keys = 0;
	if (nds->joypad & NDS_BUTTON_A)
		keys |= (1 << 0);
	if (nds->joypad & NDS_BUTTON_B)
		keys |= (1 << 1);
	if (nds->joypad & NDS_BUTTON_SELECT)
		keys |= (1 << 2);
	if (nds->joypad & NDS_BUTTON_START)
		keys |= (1 << 3);
	if (nds->joypad & NDS_BUTTON_RIGHT)
		keys |= (1 << 4);
	if (nds->joypad & NDS_BUTTON_LEFT)
		keys |= (1 << 5);
	if (nds->joypad & NDS_BUTTON_UP)
		keys |= (1 << 6);
	if (nds->joypad & NDS_BUTTON_DOWN)
		keys |= (1 << 7);
	if (nds->joypad & NDS_BUTTON_L)
		keys |= (1 << 8);
	if (nds->joypad & NDS_BUTTON_R)
		keys |= (1 << 9);
	bool enabled;
	if (keycnt & (1 << 15))
		enabled = (keys & (keycnt & 0x3FF)) == (keycnt & 0x3FF);
	else
		enabled = keys & keycnt;
	if (enabled)
	{
		mem_arm7_irq(nds->mem, 1 << 12);
		mem_arm9_irq(nds->mem, 1 << 12);
	}
}
