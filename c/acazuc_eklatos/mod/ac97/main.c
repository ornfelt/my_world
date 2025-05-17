#define ENABLE_TRACE

#include "snd.h"
#include "pci.h"

#include <errno.h>
#include <kmod.h>
#include <std.h>
#include <uio.h>

/* Intel® 82801AA (ICH) & Intel ® 82801AB
 * (ICH0) I/O Controller Hub AC ’97
 *
 *
 * Audio Codec ‘97. Revision 2.3
 *
 *
 * Intel ® I/O Controller Hub 7 (ICH7)/
 * Intel ® High Definition Audio/AC’97
 */

#define REG_NAM_RESET           0x00
#define REG_NAM_MASTER_VOLUME   0x02
#define REG_NAM_AUX_OUT_VOLUME  0x04
#define REG_NAM_MONO_VOLUME     0x06
#define REG_NAM_MASTER_TONE     0x08
#define REG_NAM_PC_BEEP_VOLUME  0x0A
#define REG_NAM_PHONE_VOLUME    0x0C
#define REG_NAM_MIC_VOLUME      0x0E
#define REG_NAM_LINE_IN_VOLUME  0x10
#define REG_NAM_CD_VOLUME       0x12
#define REG_NAM_VIDEO_VOLUME    0x14
#define REG_NAM_AUX_IN_VOLUME   0x16
#define REG_NAM_PCM_OUT_VOLUME  0x18
#define REG_NAM_RECORD_SELECT   0x1A
#define REG_NAM_RECORD_GAIN     0x1C
#define REG_NAM_RECORD_GAIN_MIC 0x1E
#define REG_NAM_GENERAL_PURPOSE 0x20
#define REG_NAM_3D_CONTROL      0x22
#define REG_NAM_AUDIO_INT_PAGE  0x24
#define REG_NAM_POWERDOWN_CTRL  0x26
#define REG_NAM_EXTENDED_AUDIO  0x28
#define REG_NAM_EXTENDED_MODEL  0x3C
#define REG_NAM_VENDOR_RSRVD1   0x5A
#define REG_NAM_PAGE_REGISTERS  0x60
#define REG_NAM_VENDOR_RSRVD2   0x70
#define REG_NAM_VENDOR_ID1      0x7C
#define REG_NAM_VENDOR_ID2      0x7E

#define REG_NABM_GLOB_CNT 0x2C /* global control */
#define REG_NABM_GLOB_STA 0x30 /* global status */
#define REG_NABM_CAS      0x34 /* codec access semaphore */
#define REG_NABM_SDM      0x80 /* sdata in map */

#define REG_NABMD_BDBAR 0x00 /* buffer descriptor base address register */
#define REG_NABMD_CIV   0x04 /* current index value */
#define REG_NABMD_LVI   0x05 /* last valid index */
#define REG_NABMD_SR    0x06 /* status */
#define REG_NABMD_PICB  0x08 /* position in current buffer */
#define REG_NABMD_PIV   0x0A /* prefetched index value */
#define REG_NABMD_CR    0x0B /* control */

#define REG_NABM_PIBD(r) (0x00 + (r))
#define REG_NABM_POBD(r) (0x10 + (r))
#define REG_NABM_MCBD(r) (0x10 + (r))

#define NABMD_CR_RPBM  (1 << 0) /* run/pause master bus */
#define NABMD_CR_RR    (1 << 1) /* reset */
#define NABMD_CR_LVBIE (1 << 2) /* last valid buffer interrupt enable */
#define NABMD_CR_FEIE  (1 << 3) /* fifo error interrupt enable */
#define NABMD_CR_IOCE  (1 << 4) /* interrupt on completion enable */

#define NABMD_SR_DCH   (1 << 0) /* DMA halted */
#define NABMD_SR_CELV  (1 << 1) /* CIV == LVI */
#define NABMD_SR_LVBCI (1 << 2) /* last valid buffer processed (BUF_DESC_F_BUP) */
#define NABMD_SR_BCIS  (1 << 3) /* buffer completion status (BUF_DESC_F_IOC) */
#define NABMD_SR_FIFOE (1 << 4) /* FIFO error */

#define BUF_DESC_F_BUP (1 << 30)
#define BUF_DESC_F_IOC (1 << 31)

#define DESC_AHEAD 2

struct buf_desc
{
	uint32_t addr;
	uint32_t size;
};

struct ac97
{
	struct pci_device *device;
	struct pci_map *nambar;
	struct pci_map *nabmbar;
	struct dma_buf *out_desc;
	struct irq_handle irq_handle;
	struct snd *out;
};

static inline uint8_t
nam_r8(struct ac97 *ac97, uint32_t reg)
{
	return pci_r8(ac97->nambar, reg);
}

static inline uint16_t
nam_r16(struct ac97 *ac97, uint32_t reg)
{
	return pci_r16(ac97->nambar, reg);
}

static inline uint8_t
nam_r32(struct ac97 *ac97, uint32_t reg)
{
	return pci_r32(ac97->nambar, reg);
}

static inline void
nam_w8(struct ac97 *ac97, uint32_t reg, uint8_t val)
{
	pci_w8(ac97->nambar, reg, val);
}

static inline void
nam_w16(struct ac97 *ac97, uint32_t reg, uint16_t val)
{
	pci_w16(ac97->nambar, reg, val);
}

static inline void
nam_w32(struct ac97 *ac97, uint32_t reg, uint32_t val)
{
	pci_w32(ac97->nambar, reg, val);
}

static inline uint8_t
nabm_r8(struct ac97 *ac97, uint32_t reg)
{
	return pci_r8(ac97->nabmbar, reg);
}

static inline uint16_t
nabm_r16(struct ac97 *ac97, uint32_t reg)
{
	return pci_r16(ac97->nabmbar, reg);
}

static inline uint8_t
nabm_r32(struct ac97 *ac97, uint32_t reg)
{
	return pci_r32(ac97->nabmbar, reg);
}

static inline void
nabm_w8(struct ac97 *ac97, uint32_t reg, uint8_t val)
{
	pci_w8(ac97->nabmbar, reg, val);
}

static inline void
nabm_w16(struct ac97 *ac97, uint32_t reg, uint16_t val)
{
	pci_w16(ac97->nabmbar, reg, val);
}

static inline void
nabm_w32(struct ac97 *ac97, uint32_t reg, uint32_t val)
{
	pci_w32(ac97->nabmbar, reg, val);
}

void
int_handler(void *userptr)
{
	struct ac97 *ac97 = userptr;
	uint16_t sr;

	sr = nabm_r16(ac97, REG_NABM_POBD(REG_NABMD_SR));
	if (!sr)
		return;
	nabm_w16(ac97, REG_NABM_POBD(REG_NABMD_SR), sr);
#if 0
	TRACE("ac97 irq: %08" PRIx32 " / %04" PRIx16,
	      nabm_w32(ac97, REG_NABM_GLOB_STA), sr);
#endif
	uint8_t lvi = nabm_r8(ac97, REG_NABM_POBD(REG_NABMD_LVI));
	uint8_t civ = nabm_r8(ac97, REG_NABM_POBD(REG_NABMD_CIV));
#if 0
	TRACE("LVI: %" PRIu8, lvi);
	TRACE("CIV: %" PRIu8, civ);
#endif
	uint8_t buf_to = (civ + DESC_AHEAD) % ac97->out->nbufs;
	while (lvi != buf_to)
	{
#if 1
		snd_fill_buf(ac97->out, ac97->out->bufs[lvi]);
		lvi = (lvi + 1) % ac97->out->nbufs;
#endif
	}
	nabm_w8(ac97, REG_NABM_POBD(REG_NABMD_LVI), lvi);
	if (sr & NABMD_SR_DCH) /* dma should always run */
		nabm_w8(ac97, REG_NABM_POBD(REG_NABMD_CR),
		         NABMD_CR_IOCE | NABMD_CR_RPBM);
}

void
ac97_free(struct ac97 *ac97)
{
	if (!ac97)
		return;
	dma_buf_free(ac97->out_desc);
	pci_unmap(ac97->device, ac97->nambar);
	pci_unmap(ac97->device, ac97->nabmbar);
	snd_free(ac97->out);
	free(ac97);
}

int
init_pci(struct pci_device *device, void *userdata)
{
	struct ac97 *ac97 = NULL;
	int ret;

	(void)userdata;
	ac97 = malloc(sizeof(*ac97), M_ZERO);
	if (!ac97)
	{
		TRACE("ac97: failed to allocate ac97");
		return -ENOMEM;
	}
	ret = snd_alloc(&ac97->out);
	if (ret)
	{
		TRACE("ac97: failed to create out snd");
		goto err;
	}
	ret = pci_map_bar(device, 0, PAGE_SIZE, 0, &ac97->nambar);
	if (ret)
	{
		TRACE("ac97: failed to init nambar");
		goto err;
	}
	ret = pci_map_bar(device, 1, PAGE_SIZE, 0, &ac97->nabmbar);
	if (ret)
	{
		TRACE("ac97: failed to init nabmbar");
		goto err;
	}
	nabm_w32(ac97, REG_NABM_GLOB_CNT, 0x3);
	nam_w16(ac97, REG_NAM_RESET, 0);
#if 0
	uint16_t vendor1 = nam_ru16(ac97, REG_VENDOR_ID1);
	uint16_t vendor2 = nam_ru16(ac97, REG_VENDOR_ID2);
	TRACE("vendor: %" PRIx16 "%" PRIx16 "%" PRIx16 "%" PRIx16,
	      (vendor1 >> 8) & 0xFF, vendor1 & 0xFF,
	      (vendor2 >> 8) & 0xFF, vendor2 & 0xFF);
	TRACE("status: %08" PRIx32, nabm_r32(ac97, REG_NABM_GLOB_STA));
#endif
	ret = dma_buf_alloc(PAGE_SIZE, DMA_32BIT, &ac97->out_desc);
	if (ret)
	{
		TRACE("ac97: descriptor allocation failed");
		goto err;
	}
	for (size_t i = 0; i < ac97->out->nbufs; ++i)
	{
		struct buf_desc *desc = &((struct buf_desc*)ac97->out_desc->data)[i];
		desc->addr = pm_page_addr(ac97->out->bufs[i]->pages);
		desc->size = PAGE_SIZE / 2;
		desc->size |= BUF_DESC_F_IOC;
	}
	nam_w16(ac97, REG_NAM_MASTER_VOLUME, 0x0000);
	nam_w16(ac97, REG_NAM_PCM_OUT_VOLUME, 0x0000);
	nabm_w8(ac97, REG_NABM_POBD(REG_NABMD_CR), NABMD_CR_RR);
	while (nabm_r8(ac97, REG_NABM_POBD(REG_NABMD_CR)) & NABMD_CR_RR) /* XXX timeout */
		;
	nabm_w32(ac97, REG_NABM_POBD(REG_NABMD_BDBAR),
	          pm_page_addr(ac97->out_desc->pages));
	nabm_w8(ac97, REG_NABM_POBD(REG_NABMD_LVI), DESC_AHEAD);
	ret = pci_register_irq(device, int_handler, ac97, &ac97->irq_handle);
	if (ret)
	{
		TRACE("ac97: failed to enable irq");
		goto err;
	}
	nabm_w8(ac97, REG_NABM_POBD(REG_NABMD_CR),
	          NABMD_CR_IOCE | NABMD_CR_RPBM);
	return 0;

err:
	ac97_free(ac97);
	return ret;
}

int
init(void)
{
	pci_probe(0x8086, 0x2415, init_pci, NULL);
	return 0;
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
	.name = "ac97",
	.init = init,
	.fini = fini,
};
