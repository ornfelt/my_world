#define ENABLE_TRACE

#include "pci.h"

#include <net/if.h>

#include <errno.h>
#include <mutex.h>
#include <waitq.h>
#include <time.h>
#include <kmod.h>
#include <std.h>
#include <mem.h>

/* RTL8139C(L)+ datasheet 1.6
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

#define REG_TSD0      0x10
#define REG_TSD1      0x14
#define REG_TSD2      0x18
#define REG_TSD3      0x1C
#define REG_TSAD0     0x20
#define REG_TSAD1     0x24
#define REG_TSAD2     0x28
#define REG_TSAD3     0x2C
#define REG_RBSTART   0x30

#define REG_ERBCR     0x34
#define REG_ERSR      0x36
#define REG_CR        0x37

#define REG_CAPR      0x38
#define REG_CBR       0x3A

#define REG_IMR       0x3C
#define REG_ISR       0x3E
#define REG_TCR       0x40
#define REG_RCR       0x44
#define REG_TCTR      0x48
#define REG_MPC       0x4C
#define REG_9346CR    0x50

#define REG_CONFIG0   0x51
#define REG_CONFIG1   0x52
#define REG_TIMERINT  0x54
#define REG_MSR       0x58
#define REG_CONFIG3   0x59
#define REG_CONFIG4   0x5A
#define REG_MULINT    0x5C
#define REG_RERID     0x5E
#define REG_TSAD      0x60
#define REG_BMCR      0x62
#define REG_BMSR      0x64
#define REG_ANAR      0x66
#define REG_ANLPAR    0x68
#define REG_ANER      0x6A
#define REG_DIS       0x6C
#define REG_FCSC      0x6E
#define REG_NWAYTR    0x70
#define REG_REC       0x72
#define REG_CSCR      0x74
#define REG_PHY1_PARM 0x78
#define REG_TW_PARM   0x7C
#define REG_PHY2_PARM 0x80
#define REG_TDOKLADDR 0x82
#define REG_CRC0      0x84
#define REG_CRC1      0x85
#define REG_CRC2      0x84
#define REG_CRC3      0x86
#define REG_CRC4      0x87
#define REG_CRC5      0x88
#define REG_CRC6      0x89
#define REG_CRC7      0x8A
#define REG_WAKEUP0   0x8C
#define REG_WAKEUP1   0x94
#define REG_WAKEUP2   0x9C
#define REG_WAKEUP3   0xA4
#define REG_WAKEUP4   0xAC
#define REG_WAKEUP5   0xB4
#define REG_WAKEUP6   0xBC
#define REG_WAKEUP7   0xC4
#define REG_LSBCRC0   0xCC
#define REG_LSBCRC1   0xCD
#define REG_LSBCRC2   0xCE
#define REG_LSBCRC3   0xCF
#define REG_LSBCRC4   0xD0
#define REG_LSBCRC5   0xD1
#define REG_LSBCRC6   0xD2
#define REG_LSBCRC7   0xD3
#define REG_FLASH     0xD4
#define REG_CONFIG5   0xD8
#define REG_TPPOLL    0xD9

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
#define IR_RBO   (1 << 4)  /* rx buffer overflow */
#define IR_PUN   (1 << 5)  /* packet underrun / link change */
#define IR_FOVW  (1 << 6)  /* rx fifo overflow */
#define IR_TDU   (1 << 7)  /* tx descriptor unavailable */
#define IR_SWINT (1 << 8)  /* software */
#define IR_LCHG  (1 << 13) /* cable length change */
#define IR_TOUT  (1 << 14) /* timeout */
#define IR_SERR  (1 << 15) /* system error */

#define RCR_AAP      (1 << 0) /* accept all packets (promiscuous) */
#define RCR_APM      (1 << 1) /* accept physical match */
#define RCR_AM       (1 << 2) /* accept multicast */
#define RCR_AB       (1 << 3) /* accept broadcast */
#define RCR_AR       (1 << 4) /* accept runt (< 64 bytes) */
#define RCR_AER      (1 << 5) /* accept error */
#define RCR_SEL      (1 << 6) /* eeprom type select (off: 9346, on: 9356) */
#define RCR_WRAP     (1 << 7) /* enable rx buf wrap mode */
#define RCR_MXDMA(n) (((n) & 0x7) << 8)  /* max dma burst size */
#define RCR_RBLEN(n) (((n) & 0x3) << 11) /* rx buffer length */
#define RCR_RXFTH(n) (((n) & 0x7) << 13) /* rx fifo threshold */
#define RCR_RER8     (1 << 16) /* rx with packet larger than 8 bytes */
#define RCR_MERINT   (1 << 17) /* multiple early interrupt select */
#define RCR_ERTH(n)  (((n) & 0xF) << 24) /* early rx threshold bits */

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

#define TSD_SIZE(n) ((n & 0x1FF))        /* packet size */
#define TSD_OWN     (1 << 13)            /* DMA operation completed (desc available) */
#define TSD_TUN     (1 << 14)            /* tx fifo exhausted */
#define TSD_TOK     (1 << 15)            /* transmit ok */
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

#define TCR_MXDMA(n)   ((n) << 8)  /* max dma burst size */
#define TCR_CRC        (1 << 16)   /* append CRC */
#define TCR_LBK(n)     ((n) << 17) /* loopback test */
#define TCR_IFG2       (1 << 19)   /* interface gap 2 */
#define TCR_HWVERIDI1  (1 << 23)   /* hardware version id1 */
#define TCR_IFG01(n)   ((n) << 24) /* interframe gap */
#define TCR_HWVERID0(n) (((n) >> 26) & 0x1F) /* hardware version id0 */

#define MSR_RXPF     (1 << 0) /* receive pause flag */
#define MSR_TXPF     (1 << 1) /* transsmit pause flag */
#define MSR_LINKB    (1 << 2) /* link status */
#define MSR_SPEED_10 (1 << 3) /* 10Mbps if enable, 100Mbps otherwise */
#define MSR_AUX_STS  (1 << 4) /* aux power present status */
#define MSR_RXFCE    (1 << 6) /* receive flow control enable */
#define MSR_TXFCE    (1 << 7) /* transmit flow control enable */

#define RX_NPAGES 9
#define TX_NDESC  4

struct rtl8139
{
	struct pci_device *device;
	struct pci_map *pci_map;
	struct dma_buf *rxb;
	uint32_t rxb_off;
	uint32_t rxb_len;
	struct dma_buf *txb[TX_NDESC];
	size_t tx_len[TX_NDESC];
	uint32_t tx_head;
	uint32_t tx_tail;
	struct waitq waitq;
	struct mutex mutex;
	struct irq_handle irq_handle;
	struct netif *netif;
};

static inline uint8_t
rtl_r8(struct rtl8139 *rtl, uint32_t reg)
{
	return pci_r8(rtl->pci_map, reg);
}

static inline uint16_t
rtl_r16(struct rtl8139 *rtl, uint32_t reg)
{
	return pci_r16(rtl->pci_map, reg);
}

static inline uint32_t
rtl_r32(struct rtl8139 *rtl, uint32_t reg)
{
	return pci_r32(rtl->pci_map, reg);
}

static inline void
rtl_w8(struct rtl8139 *rtl, uint32_t reg, uint8_t val)
{
	pci_w8(rtl->pci_map, reg, val);
}

static inline void
rtl_w16(struct rtl8139 *rtl, uint32_t reg, uint16_t val)
{
	pci_w16(rtl->pci_map, reg, val);
}

static inline void
rtl_w32(struct rtl8139 *rtl, uint32_t reg, uint32_t val)
{
	pci_w32(rtl->pci_map, reg, val);
}

static inline void
rtl_lock(struct rtl8139 *rtl)
{
	mutex_lock(&rtl->mutex);
}

static inline void
rtl_unlock(struct rtl8139 *rtl)
{
	mutex_unlock(&rtl->mutex);
}

static int
emit_pkt(struct netif *netif, struct netpkt *pkt)
{
	struct rtl8139 *rtl = netif->userdata;
	int ret;

	if (pkt->len >= 0x700)
		return -ENOBUFS;
	rtl_lock(rtl);
	while (!(rtl_r32(rtl, REG_TSD0 + rtl->tx_tail * 4) & TSD_OWN))
	{
		ret = waitq_wait_tail_mutex(&rtl->waitq, &rtl->mutex, NULL);
		if (ret)
		{
			rtl_unlock(rtl);
			return ret;
		}
	}
	memcpy(rtl->txb[rtl->tx_tail]->data, pkt->data, pkt->len);
	rtl_w32(rtl, REG_TSAD0 + rtl->tx_tail * 4,
	         pm_page_addr(rtl->txb[rtl->tx_tail]->pages));
	rtl_w32(rtl, REG_TSD0 + rtl->tx_tail * 4, pkt->len);
	rtl->tx_len[rtl->tx_tail] = pkt->len;
	rtl->tx_tail = (rtl->tx_tail + 1) % TX_NDESC;
	rtl_unlock(rtl);
	return 0;
}

static void
update_rx_filter(struct netif *netif)
{
	struct rtl8139 *rtl = netif->userdata;
	uint32_t rcr;

	rcr = RCR_WRAP | RCR_RXFTH(7) | RCR_RBLEN(3);
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

static const struct netif_op
netif_op =
{
	.emit = emit_pkt,
	.update_rx_filter = update_rx_filter,
};

static void
rx_pkt(struct rtl8139 *rtl)
{
	uint8_t *pkt = &((uint8_t*)rtl->rxb->data)[rtl->rxb_off];
	uint16_t hdr = ((uint16_t*)pkt)[0];
	uint16_t len = ((uint16_t*)pkt)[1];
	uint8_t *data = &pkt[4];
	if (hdr & (PKT_FAE | PKT_CRC | PKT_ISE))
	{
		rtl->netif->stats.rx_errors++;
		goto end;
	}
	if (!(hdr & PKT_ROK))
	{
		rtl->netif->stats.rx_errors++;
		goto end;
	}
	rtl->netif->stats.rx_packets++;
	rtl->netif->stats.rx_bytes += len;
	struct netpkt *netpkt = netpkt_alloc(len);
	if (!netpkt)
	{
		TRACE("rtl819: failed to allocate packet");
		goto end;
	}
	memcpy(netpkt->data, data, len);
	ether_input(rtl->netif, netpkt);

end:
	rtl->rxb_off = ((rtl->rxb_off + len + 4 + 3) & ~3) % rtl->rxb_len;
	rtl_w32(rtl, REG_CAPR, rtl->rxb_off - 0x10);
}

static void
int_handler(void *userdata)
{
	struct rtl8139 *rtl = userdata;
	uint16_t status;

	status = rtl_r16(rtl, REG_ISR);
	if (!status)
		return;
	rtl_w16(rtl, REG_ISR, status);
	if (status & IR_PUN)
	{
		if (rtl_r8(rtl, REG_MSR) & MSR_LINKB)
			rtl->netif->flags &= ~IFF_RUNNING;
		else
			rtl->netif->flags |= IFF_RUNNING;
	}
	if (status & (IR_ROK | IR_RER))
		rx_pkt(rtl);
	if (status & (IR_TOK | IR_TER))
	{
		while (rtl->tx_head != rtl->tx_tail)
		{
			if (rtl_r32(rtl, REG_TSD0 + rtl->tx_head * 4) & TSD_TOK)
			{
				rtl->netif->stats.tx_packets++;
				rtl->netif->stats.tx_bytes += rtl->tx_len[rtl->tx_head];
			}
			else
			{
				rtl->netif->stats.tx_errors++;
			}
			rtl->tx_head = (rtl->tx_head + 1) % TX_NDESC;
		}
		waitq_broadcast(&rtl->waitq, 0);
	}
}

void
rtl8139_free(struct rtl8139 *rtl)
{
	if (!rtl)
		return;
	for (size_t i = 0; i < TX_NDESC; ++i)
		dma_buf_free(rtl->txb[i]);
	dma_buf_free(rtl->rxb);
	netif_free(rtl->netif);
	waitq_destroy(&rtl->waitq);
	mutex_destroy(&rtl->mutex);
	pci_unmap(rtl->device, rtl->pci_map);
	free(rtl);
}

int
init_pci(struct pci_device *device, void *userdata)
{
	struct rtl8139 *rtl;
	int ret;

	(void)userdata;
	rtl = malloc(sizeof(*rtl), M_ZERO);
	if (!rtl)
	{
		TRACE("rtl8139: rtl allocation failed");
		return -ENOMEM;
	}
	ret = netif_alloc("eth", &netif_op, &rtl->netif);
	if (ret)
	{
		TRACE("rtl8139: netif creation failed");
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
		TRACE("rtl8139: failed to init bar0");
		goto err;
	}
	rtl_w8(rtl, REG_CONFIG1, 0x0);
	rtl_w8(rtl, REG_CR, CR_RST);
	while (rtl_r8(rtl, REG_CR) & CR_RST) /* XXX timeout */
		;
	rtl->netif->ether.addr[0] = rtl_r8(rtl, REG_IDR0);
	rtl->netif->ether.addr[1] = rtl_r8(rtl, REG_IDR1);
	rtl->netif->ether.addr[2] = rtl_r8(rtl, REG_IDR2);
	rtl->netif->ether.addr[3] = rtl_r8(rtl, REG_IDR3);
	rtl->netif->ether.addr[4] = rtl_r8(rtl, REG_IDR4);
	rtl->netif->ether.addr[5] = rtl_r8(rtl, REG_IDR5);
	ret = dma_buf_alloc(RX_NPAGES * PAGE_SIZE, DMA_32BIT, &rtl->rxb);
	if (ret)
	{
		TRACE("rtl8139: rxb allocation failed");
		goto err;
	}
	rtl->rxb_len = 1024 * 64;
	for (size_t i = 0; i < TX_NDESC; ++i)
	{
		ret = dma_buf_alloc(PAGE_SIZE, DMA_32BIT, &rtl->txb[i]);
		if (ret)
		{
			TRACE("rtl8139: tx allocation failed");
			goto err;
		}
	}
	rtl_w32(rtl, REG_RBSTART, pm_page_addr(rtl->rxb->pages));
	rtl_w16(rtl, REG_IMR, IR_ROK | IR_TOK | IR_RER | IR_TER | IR_PUN);
	update_rx_filter(rtl->netif);
	ret = pci_register_irq(device, int_handler, rtl, &rtl->irq_handle);
	if (ret)
	{
		TRACE("rtl8139: failed to enable irq");
		goto err;
	}
	rtl_w8(rtl, REG_CR, CR_RE | CR_TE);
	if (!(rtl_r8(rtl, REG_MSR) & MSR_LINKB))
		rtl->netif->flags |= IFF_RUNNING;
	return 0;

err:
	rtl8139_free(rtl);
	return ret;
}

int
init(void)
{
	pci_probe(0x10EC, 0x8139, init_pci, NULL);
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
	.name = "rtl8139",
	.init = init,
	.fini = fini,
};
