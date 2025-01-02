#define ENABLE_TRACE

#include "pci.h"

#include <net/if.h>

#include <errno.h>
#include <kmod.h>
#include <std.h>
#include <mem.h>

/* RTL8169 1.21
 */

#define REG_IDR0      0x00
#define REG_IDR1      0x01
#define REG_IDR2      0x02
#define REG_IDR3      0x03
#define REG_IDR4      0x04
#define REG_IDR5      0x05
#define REG_MAR0      0x08
#define REG_MAR1      0x09
#define REG_MAR2      0x0A
#define REG_MAR3      0x0B
#define REG_MAR4      0x0C
#define REG_MAR5      0x0D
#define REG_MAR6      0x0E
#define REG_MAR7      0x0F

#define REG_DTCCR     0x10
#define REG_TNPDS     0x20
#define REG_THPDS     0x28
#define REG_FLASH     0x30

#define REG_ERBCR     0x34
#define REG_ERSR      0x36
#define REG_CR        0x37

#define REG_TPPOLL    0x38

#define REG_IMR       0x3C
#define REG_ISR       0x3E
#define REG_TCR       0x40
#define REG_RCR       0x44
#define REG_TCTR      0x48
#define REG_MPC       0x4C
#define REG_9346CR    0x50

#define REG_CONFIG0   0x51
#define REG_CONFIG1   0x52
#define REG_CONFIG2   0x53
#define REG_CONFIG3   0x54
#define REG_CONFIG4   0x55
#define REG_CONFIG5   0x56
#define REG_TIMERINT  0x58
#define REG_MULINT    0x5C
#define REG_PHYAR     0x60
#define REG_TBICSR0   0x64
#define REG_TBI_ANAR  0x68
#define REG_TBI_LPAR  0x6A
#define REG_PHYSTATUS 0x6C
#define REG_WAKEUP0   0x84
#define REG_WAKEUP1   0x8C
#define REG_WAKEUP2LD 0x94
#define REG_WAKEUP2HD 0x9C
#define REG_WAKEUP3LD 0xA4
#define REG_WAKEUP3HD 0xAC
#define REG_WAKEUP4LD 0xB4
#define REG_WAKEUP5HD 0xBC
#define REG_CRC0      0xC4
#define REG_CRC1      0xC6
#define REG_CRC2      0xC8
#define REG_CRC3      0xCA
#define REG_CRC4      0xCC
#define REG_RMS       0xDA

#define REG_C_CR      0xE0
#define REG_RDSAR     0xE4
#define REG_ETTHR     0xEC
#define REG_FER       0xF0
#define REG_FEMR      0xF4
#define REG_FPSR      0xF8
#define REG_FFER      0xFC

#define IR_ROK   (1 << 0)  /* rx ok */
#define IR_RER   (1 << 1)  /* rx error */
#define IR_TOK   (1 << 2)  /* tx ok */
#define IR_TER   (1 << 3)  /* tx error */
#define IR_RDU   (1 << 4) /* rx buffer overflow / rx desc unavailable */
#define IR_PUN   (1 << 5)  /* packet underrun / link change */
#define IR_FOVW  (1 << 6)  /* rx fifo overflow */
#define IR_TDU   (1 << 7)  /* tx descriptor unavailable */
#define IR_SWINT (1 << 8)  /* software */
#define IR_TOUT  (1 << 14) /* timeout */
#define IR_SERR  (1 << 15) /* system error */

#define RCR_AAP      (1 << 0) /* accept all packets (promiscuous) */
#define RCR_APM      (1 << 1) /* accept physical match */
#define RCR_AM       (1 << 2) /* accept multicast */
#define RCR_AB       (1 << 3) /* accept broadcast */
#define RCR_AR       (1 << 4) /* accept runt (< 64 bytes) */
#define RCR_AER      (1 << 5) /* accept error */
#define RCR_SEL      (1 << 6) /* eeprom type select (off: 9346, on: 9356) */
#define RCR_MXDMA(n) (((n) & 0x7) << 8)  /* max dma burst size */
#define RCR_RXFTH(n) (((n) & 0x7) << 13) /* rx fifo threshold */
#define RCR_RER8     (1 << 16) /* rx with packet larger than 8 bytes */
#define RCR_MERINT   (1 << 24)

#define CR_BUFE (1 << 0) /* empty buffer */
#define CR_TE   (1 << 2) /* transmitter enable */
#define CR_RE   (1 << 3) /* receiver enable */
#define CR_RST  (1 << 4) /* reset */

#define PKT_ROK  (1 << 0)  /* receive ok */
#define PKT_FAE  (1 << 1)  /* frame alignment error */
#define PKT_CRC  (1 << 2)  /* CRC error */
#define PKT_LONG (1 << 3)  /* long packet (> 4k) */
#define PKT_RUNT (1 << 4)  /* runt packet (< 64 bytes) */
#define PKT_ISE  (1 << 5)  /* invalid symbol */
#define PKT_BAR  (1 << 13) /* broadcast address received */
#define PKT_PAM  (1 << 14) /* physical address matched */
#define PKT_MAR  (1 << 15) /* multicast address received */

#define TSD_SIZE(n)   ((n & 0x1FF))        /* packet size */
#define TSD_OWN       (1 << 13)            /* DMA operation completed (desc available) */
#define TSD_TUN       (1 << 14)            /* tx fifo exhausted */
#define TSD_TOK       (1 << 15)            /* transmit ok */
#define TSD_ERTXTH(n) ((1 << 16) << (n)) /* early tx threshold */
#define TSD_NCC(n)    ((1 << 24) << (n)) /* collisions count */
#define TSD_CDH       (1 << 28)          /* cd heart beat */
#define TSD_OWC       (1 << 29)          /* out of window collision */
#define TSD_TABT      (1 << 30)          /* transmit abort */
#define TSD_CRS       (1 << 31)          /* carrier sense lost (ACAB nonobstant) */

#define CR_UNLOCK 0xC0 /* unlock config registers */
#define CR_LOCK   0x00 /* lock config registers */

/* TX command byte */
#define DESC_TX_OWN         (1 << 31) /* ownership (1 = ready for card to DMA) */
#define DESC_TX_EOR         (1 << 30) /* end of ring (last of descriptor) */
#define DESC_TX_FS          (1 << 29) /* first segment */
#define DESC_TX_LS          (1 << 28) /* last segment */
#define DESC_TX_LGSEN       (1 << 27) /* large send */
#define DESC_TX_IPCS        (1 << 18) /* ip checksum offload (LGSEN = 0) */
#define DESC_TX_UDPCS       (1 << 17) /* udp checksum offload (LGSEN = 0) */
#define DESC_TX_TCPCS       (1 << 16) /* tcp checksum offload (LGSEN = 0) */
#define DESC_TX_MSS(n)      (((n) & 0x3FF) << 16) /* mss (LGSEN = 1) */
#define DESC_TX_LEN(n)      ((n) & 0xFFFF) /* desc len */
/* TX vlan byte */
#define DESC_TX_TAGC        (1 << 17) /* vlang tag control enable bit */
#define DESC_TX_VLAN_TAG(n) ((n) & 0xFFFF) /* vlan tag */

/* RX command byte */
#define DESC_RX_OWN         (1 << 31) /* ownership (1 = ready for card to DMA) */
#define DESC_RX_EOR         (1 << 30) /* end of ring (last of descriptor) */
#define DESC_RX_FS          (1 << 29) /* first segment */
#define DESC_RX_LS          (1 << 28) /* last segment */
#define DESC_RX_MAR         (1 << 27) /* multicast packet */
#define DESC_RX_PAM         (1 << 26) /* physical address match */
#define DESC_RX_BAR         (1 << 25) /* broadcast address */
#define DESC_RX_BOVF        (1 << 24) /* buffer overflow */
#define DESC_RX_FOVF        (1 << 23) /* fifo overflow */
#define DESC_RX_RWT         (1 << 22) /* watchdog timer */
#define DESC_RX_RES         (1 << 21) /* receive error sum */
#define DESC_RX_RUNT        (1 << 20) /* runt packet (< 64 bytes) */
#define DESC_RX_CRC         (1 << 19) /* crc error */
#define DESC_RX_PID(n)      (((n) >> 17) & 0x3) /* protocol id */
#define DESC_RX_IPF         (1 << 16) /* ip checksum failure */
#define DESC_RX_UDPF        (1 << 15) /* udp checksum failure */
#define DESC_RX_TCPF        (1 << 14) /* tcp checksum failure */
#define DESC_RX_LEN(n)      ((n) & 0x3FFF) /* desc len */
/* RX vlan byte */
#define DESC_RX_TAVA        (1 << 16) /* vlan tag available */
#define DESC_RX_VLAN_TAG(n) ((n) & 0xFFFF) /* vlan tag */

#define TPPOLL_HPQ    (1 << 7) /* high priority queue */
#define TPPOLL_NPQ    (1 << 6) /* normal priority queue */
#define TPPOLL_FSWINT (1 << 0) /* forced software interrupt */

#define TCR_MXDMA(n)    ((n) << 8)  /* max dma burst size */
#define TCR_CRC         (1 << 16)   /* append CRC */
#define TCR_LBK(n)      ((n) << 17) /* loopback test */
#define TCR_IFG2        (1 << 19)   /* interface gap 2 */
#define TCR_HWVERIDI1   (1 << 23)   /* hardware version id1 */
#define TCR_IFG01(n)    ((n) << 24) /* interframe gap */
#define TCR_HWVERID0(n) (((n) >> 26) & 0x1F) /* hardware version id0 */

#define PHYSTATUS_FD    (1 << 0) /* full duplex */
#define PHYSTATUS_LS    (1 << 1) /* link status */
#define PHYSTATUS_10M   (1 << 2) /* 10Mbps link */
#define PHYSTATUS_100M  (1 << 3) /* 100Mbps link */
#define PHYSTATUS_1000M (1 << 4) /* 1000Mbps link */
#define PHYSTATUS_RXF   (1 << 5) /* receive flow control */
#define PHYSTATUS_TXF   (1 << 6) /* transmit flow control */
#define PHYSTATUS_TBI   (1 << 7) /* TBI enable */

#define DESC_LEN   0xFFF
#define DESC_COUNT 1024

struct rtl8169_desc
{
	uint32_t command;
	uint32_t vlan;
	uint64_t buffer;
};

struct rtl8169
{
	struct pci_device *device;
	struct pci_map *pci_map;
	struct dma_buf *rxd;
	struct dma_buf *txd;
	struct dma_buf *rxb[DESC_COUNT];
	struct dma_buf *txb[DESC_COUNT];
	struct waitq waitq;
	struct mutex mutex;
	struct netif *netif;
	size_t txd_tail;
	size_t rxd_tail;
	struct irq_handle irq_handle;
};

static inline uint8_t rtl_r8(struct rtl8169 *rtl, uint32_t reg)
{
	return pci_r8(rtl->pci_map, reg);
}

static inline uint16_t rtl_r16(struct rtl8169 *rtl, uint32_t reg)
{
	return pci_r16(rtl->pci_map, reg);
}

static inline uint32_t rtl_r32(struct rtl8169 *rtl, uint32_t reg)
{
	return pci_r32(rtl->pci_map, reg);
}

static inline void rtl_w8(struct rtl8169 *rtl, uint32_t reg, uint8_t val)
{
	pci_w8(rtl->pci_map, reg, val);
}

static inline void rtl_w16(struct rtl8169 *rtl, uint32_t reg, uint16_t val)
{
	pci_w16(rtl->pci_map, reg, val);
}

static inline void rtl_w32(struct rtl8169 *rtl, uint32_t reg, uint32_t val)
{
	pci_w32(rtl->pci_map, reg, val);
}

static inline void rtl_lock(struct rtl8169 *rtl)
{
	mutex_lock(&rtl->mutex);
}

static inline void rtl_unlock(struct rtl8169 *rtl)
{
	mutex_unlock(&rtl->mutex);
}

static int emit_pkt(struct netif *netif, struct netpkt *pkt)
{
	struct rtl8169 *rtl = netif->userdata;
	struct rtl8169_desc *txd;
	struct dma_buf *txb;
	int ret;

	if (pkt->len >= DESC_LEN)
		return -ENOBUFS;
	rtl_lock(rtl);
	while (1)
	{
		txd = &((struct rtl8169_desc*)rtl->txd->data)[rtl->txd_tail];
		if (!(txd->command & DESC_TX_OWN))
			break;
		ret = waitq_wait_tail_mutex(&rtl->waitq, &rtl->mutex, NULL);
		if (ret)
		{
			rtl_unlock(rtl);
			return ret;
		}
	}
	txb = rtl->txb[rtl->txd_tail];
	memcpy(txb->data, pkt->data, pkt->len);
	txd->buffer = pm_page_addr(txb->pages);
	txd->vlan = 0;
	txd->command = pkt->len | DESC_TX_FS | DESC_TX_LS | DESC_TX_OWN;
	if (rtl->txd_tail == DESC_COUNT - 1)
		txd->command |= DESC_TX_EOR;
	rtl_w8(rtl, REG_TPPOLL, TPPOLL_NPQ);
	while (rtl_r8(rtl, REG_TPPOLL) & TPPOLL_NPQ)
		;
	rtl->netif->stats.tx_packets++;
	rtl->netif->stats.tx_bytes += pkt->len;
	rtl->txd_tail = (rtl->txd_tail + 1) % DESC_COUNT;
	rtl_unlock(rtl);
	return 0;
}

static void update_rx_filter(struct netif *netif)
{
	struct rtl8169 *rtl = netif->userdata;
	uint32_t rcr;

	rcr = RCR_RXFTH(7) | RCR_MXDMA(7);
	if (netif->flags & IFF_UP)
	{
		rcr |= RCR_APM;
		if (netif->flags & IFF_BROADCAST)
			rcr |= RCR_AB;
		if (netif->flags & IFF_MULTICAST)
			rcr |= RCR_AM;
		if (netif->flags & (IFF_PROMISC | IFF_ALLMULTI))
			rcr |= RCR_AAP;
	}
	rtl_w32(rtl, REG_RCR, rcr);
}

static const struct netif_op netif_op =
{
	.emit = emit_pkt,
	.update_rx_filter = update_rx_filter,
};

static void rx_desc(struct rtl8169 *rtl, struct rtl8169_desc *desc, size_t i)
{
	if (desc->command & (DESC_RX_BOVF | DESC_RX_FOVF | DESC_RX_RWT | DESC_RX_RES | DESC_RX_CRC))
	{
		rtl->netif->stats.rx_errors++;
		return;
	}
	size_t len = DESC_RX_LEN(desc->command);
	rtl->netif->stats.rx_packets++;
	rtl->netif->stats.rx_bytes += len;
	struct netpkt *netpkt = netpkt_alloc(len);
	if (!netpkt)
		panic("rtl8169: failed to allocate packet\n");
	memcpy(netpkt->data, rtl->rxb[i]->data, len);
	ether_input(rtl->netif, netpkt);
}

static void rx_pkt(struct rtl8169 *rtl)
{
	while (1)
	{
		struct rtl8169_desc *desc = &((struct rtl8169_desc*)rtl->rxd->data)[rtl->rxd_tail];
		if (desc->command & DESC_RX_OWN)
			return;
		rx_desc(rtl, desc, rtl->rxd_tail);
		desc->vlan = 0;
		desc->buffer = pm_page_addr(rtl->rxb[rtl->rxd_tail]->pages);
		desc->command = DESC_RX_OWN | DESC_LEN;
		if (rtl->rxd_tail == DESC_COUNT - 1)
			desc->command |= DESC_RX_EOR;
		rtl->rxd_tail = (rtl->rxd_tail + 1) % DESC_COUNT;
	}
}

static void int_handler(void *userdata)
{
	struct rtl8169 *rtl = userdata;
	uint16_t status;

	status = rtl_r16(rtl, REG_ISR);
	if (!status)
		return;
#if 0
	TRACE("rtl int %04" PRIx16, status);
#endif
	rtl_w16(rtl, REG_ISR, status);
	if (status & IR_PUN)
	{
		if (rtl_r8(rtl, REG_PHYSTATUS) & PHYSTATUS_LS)
			rtl->netif->flags |= IFF_RUNNING;
		else
			rtl->netif->flags &= ~IFF_RUNNING;
	}
	if (status & (IR_ROK | IR_RER))
		rx_pkt(rtl);
	if (status & (IR_TOK | IR_TER))
		waitq_broadcast(&rtl->waitq, 0);
}

static int alloc_desc(struct dma_buf **descs, struct dma_buf **bufs, int rx)
{
	int ret;

	ret = dma_buf_alloc(sizeof(struct rtl8169_desc) * DESC_COUNT, 0, descs);
	if (ret)
	{
		TRACE("rtl8169: descriptor allocation failed");
		return -ENOMEM;
	}
	for (size_t i = 0; i < DESC_COUNT; ++i)
	{
		ret = dma_buf_alloc(PAGE_SIZE, 0, &bufs[i]);
		if (ret)
		{
			TRACE("rtl8169: buffer allocation failed");
			return -ENOMEM;
		}
		struct rtl8169_desc *desc = &((struct rtl8169_desc*)(*descs)->data)[i];
		desc->vlan = 0;
		desc->buffer = pm_page_addr(bufs[i]->pages);
		if (i == DESC_COUNT - 1)
			desc->command |= DESC_RX_EOR;
		else
			desc->command = 0;
		if (rx)
			desc->command |= DESC_LEN | DESC_RX_OWN;
	}
	return 0;
}

void rtl8169_free(struct rtl8169 *rtl)
{
	if (!rtl)
		return;
	netif_free(rtl->netif);
	for (size_t i = 0; i < DESC_COUNT; ++i)
	{
		dma_buf_free(rtl->rxb[i]);
		dma_buf_free(rtl->txb[i]);
	}
	dma_buf_free(rtl->rxd);
	dma_buf_free(rtl->txd);
	waitq_destroy(&rtl->waitq);
	mutex_destroy(&rtl->mutex);
	pci_unmap(rtl->device, rtl->pci_map);
	free(rtl);
}

int init_pci(struct pci_device *device, void *userdata)
{
	struct rtl8169 *rtl;
	int ret;

	(void)userdata;
	rtl = malloc(sizeof(*rtl), M_ZERO);
	if (!rtl)
	{
		TRACE("rtl8169: rtl allocation failed");
		return -ENOMEM;
	}
	ret = netif_alloc("eth", &netif_op, &rtl->netif);
	if (ret)
	{
		TRACE("rtl8169: netif creation failed");
		goto err;
	}
	rtl->netif->flags = IFF_UP | IFF_BROADCAST;
	rtl->netif->userdata = rtl;
	waitq_init(&rtl->waitq);
	mutex_init(&rtl->mutex, 0);
	rtl->device = device;
	ret = pci_map_bar(device, 0, PAGE_SIZE, 0, &rtl->pci_map);
	if (ret)
	{
		TRACE("rtl8169: failed to init bar0");
		goto err;
	}
	rtl_w8(rtl, REG_CR, CR_RST);
	while (rtl_r8(rtl, REG_CR) & CR_RST) /* XXX timeout */
		;
	rtl->netif->ether.addr[0] = rtl_r8(rtl, REG_IDR0);
	rtl->netif->ether.addr[1] = rtl_r8(rtl, REG_IDR1);
	rtl->netif->ether.addr[2] = rtl_r8(rtl, REG_IDR2);
	rtl->netif->ether.addr[3] = rtl_r8(rtl, REG_IDR3);
	rtl->netif->ether.addr[4] = rtl_r8(rtl, REG_IDR4);
	rtl->netif->ether.addr[5] = rtl_r8(rtl, REG_IDR5);
	ret = alloc_desc(&rtl->rxd, rtl->rxb, 1);
	if (ret)
		goto err;
	ret = alloc_desc(&rtl->txd, rtl->txb, 0);
	if (ret)
		goto err;
	rtl_w8(rtl, REG_9346CR, CR_UNLOCK);
	update_rx_filter(rtl->netif);
	rtl_w32(rtl, REG_RDSAR + 0, pm_page_addr(rtl->rxd->pages));
	rtl_w32(rtl, REG_RDSAR + 4, pm_page_addr(rtl->rxd->pages) >> 32);
	rtl_w16(rtl, REG_RMS, DESC_LEN);
	rtl_w8(rtl, REG_CR, CR_TE);
	rtl_w32(rtl, REG_TCR, TCR_IFG01(3) | TCR_MXDMA(0x7));
	rtl_w32(rtl, REG_TNPDS, pm_page_addr(rtl->txd->pages));
	rtl_w8(rtl, REG_ETTHR, 0x1);
	ret = pci_register_irq(device, int_handler, rtl, &rtl->irq_handle);
	if (ret)
	{
		TRACE("rtl8169: failed to enable IRQ");
		goto err;
	}
	rtl_w8(rtl, REG_CR, CR_RE | CR_TE);
	rtl_w16(rtl, REG_IMR, IR_ROK | IR_TOK | IR_RER | IR_TER | IR_PUN);
	rtl_w8(rtl, REG_9346CR, CR_LOCK);
	if (rtl_r8(rtl, REG_PHYSTATUS) & PHYSTATUS_LS)
		rtl->netif->flags |= IFF_RUNNING;
	return 0;

err:
	rtl8169_free(rtl);
	return ret;
}

int init(void)
{
	pci_probe(0x10EC, 0x8168, init_pci, NULL);
	return 0;
}

void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "rtl8169",
	.init = init,
	.fini = fini,
};
