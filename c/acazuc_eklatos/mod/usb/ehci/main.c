#define ENABLE_TRACE

#include "usb.h"
#include "pci.h"

#include <kmod.h>

/* Enhanced Host Controller Interface Specification for Universal Serial Bus
 * Revision 1.0
 */

#define REG_USBCMD           (0x00) /* USB command */
#define REG_USBSTS           (0x04) /* USB status */
#define REG_USBINTR          (0x08) /* USB interrupt enable */
#define REG_FRINDEX          (0x0C) /* USB frame index */
#define REG_CTRLDSSEGMENT    (0x10) /* 4G Segment Selector */
#define REG_PERIODICLISTBASE (0x14) /* Frame List Base Address */
#define REG_ASYNCLISTADDR    (0x18) /* Next Asynchronous List Address */
#define REG_CONFIGFLAG       (0x40) /* Configured Flag Register */
#define REG_PORTSC(n)        (0x44 + (n) * 0x4) /* Port Status/Control */

#define CMD_RS       (1 << 0) /* run / stop */
#define CMD_HCRESET  (1 << 1) /* host controller reset */
#define CMD_FLS(n)   ((n) << 2) /* frame list size */
#define CMD_PSE      (1 << 4) /* periodic schedule enable */
#define CMD_ASE      (1 << 5) /* asnchronous schedule enable */
#define CMD_IAAD     (1 << 6) /* interrupt on async advance doorbell */
#define CMD_LHCR     (1 << 7) /* light host controller reset */
#define CMD_ASPMC(n) ((n) << 8) /* asynchronous schedule park mode count */
#define CMD_ASPME    (1 << 11) /* asynchronous schedule park mode enable */
#define CMD_ITC(n)   ((n) << 16) /* interrupt threshold control */

#define STS_USBINT    (1 << 0) /* USB interrupt */
#define STS_USBERRINT (1 << 1) /* USB error interrupt */
#define STS_PCD       (1 << 2) /* port change detect */
#define STS_FLR       (1 << 3) /* frame list rollover */
#define STS_HSE       (1 << 4) /* host system error */
#define STS_IAA       (1 << 5) /* interrupt on async advance */
#define STS_HCH       (1 << 12) /* HC halted */
#define STS_RE        (1 << 13) /* reclamation */
#define STS_PSS       (1 << 14) /* periodic schedule status */
#define STS_ASS       (1 << 15) /* asynchronous schedule status */

#define PORTSC_CCS    (1 << 0) /* current connect status */
#define PORTSC_CSC    (1 << 1) /* connect status change */
#define PORTSC_PE     (1 << 2) /* port enable */
#define PORTSC_PEC    (1 << 3) /* port enable change */
#define PORTSC_OCA    (1 << 4) /* over-current active */
#define PORTSC_OCC    (1 << 5) /* over-current change */
#define PORTSC_FPR    (1 << 6) /* force port resume */
#define PORTSC_SUS    (1 << 7) /* suspend */
#define PORTSC_PR     (1 << 8) /* port reset */
#define PORTSC_LST(n) ((n) << 10) /* line status */
#define PORTSC_PP     (1 << 12) /* port power */
#define PORTSC_PO     (1 << 13) /* port owner */
#define PORTSC_PIC(n) ((n) << 14) /* port indicator control */
#define PORTSC_PTC(n) ((n) << 16) /* port test control */
#define PORTSC_WCE    (1 << 20) /* wake on connect enable */
#define PORTSC_WDE    (1 << 21) /* wake on disconnect enable */
#define PORTSC_WOE    (1 << 22) /* wake on over-current enable */

#define ITD_NEXT_T    (1 << 0) /* terminate */
#define ITD_NEXT_ITD  (0 << 1) /* next is itd */
#define ITD_NEXT_QH   (1 << 1) /* next is qh */
#define ITD_NEXT_SITD (2 << 1) /* next is sitd */
#define ITD_NEXT_FSTN (3 << 1) /* next is fstn */

#define ITD_OFF_PG(n) ((n) << 12) /* page select */
#define ITD_OFF_IOC   (1 << 15) /* interrupit on completion */

#define ITD_LEN_ERR    (1 << 12) /* transaction error */
#define ITD_LEN_BABBLE (1 << 13) /* babble detected */
#define ITD_LEN_DBE    (1 << 14) /* data buffer error */
#define ITD_LEN_ACTIVE (1 << 15) /* active */

#define QTD_NEXT_T (1 << 0) /* terminate */

#define QTD_STS_PS      (1 << 0) /* ping state */
#define QTD_STS_STS     (1 << 1) /* split transaction state */
#define QTD_STS_MMF     (1 << 2) /* missed micro-frame */
#define QTD_STS_TE      (1 << 3) /* transaction error */
#define QTD_STS_BABBLE  (1 << 4) /* babble detected */
#define QTD_STS_DBE     (1 << 5) /* data buffer error */
#define QTD_STS_HALTED  (1 << 6) /* halted */
#define QTD_STS_ACTIVE  (1 << 7) /* active */
#define QTD_STS_PID(n)  ((n) << 8) /* packet id */
#define QTD_STS_CERR(n) ((n) << 10) /* error counter */
#define QTD_STS_CP(n)   ((n) << 12) /* current page */
#define QTD_STS_IOC     (1 << 15) /* interrupt on completion */

#define QTD_LEN_DT (1 << 15) /* data toggle */

#define QH_LINK_T    (1 << 0) /* terminate */
#define QH_LINK_ITD  (0 << 1) /* next is itd */
#define QH_LINK_QH   (1 << 1) /* next is qh */
#define QH_LINK_SITD (2 << 1) /* next is sitd */
#define QH_LINK_FSTN (3 << 1) /* next is fstn */

#define QH_STS1_DA(n)  ((n) << 0) /* device address */
#define QH_STS1_I      (1 << 7) /* inactive on next transaction */
#define QH_STS1_EN(n)  ((n) << 8) /* endpoint number */
#define QH_STS1_EPS(n) ((n) << 12) /* endpoint speed */
#define QH_STS1_DTC    (1 << 14) /* data toggle control */
#define QH_STS1_H      (1 << 15) /* head of reclamation list flag */
#define QH_STS1_MPL(n) ((n) << 16) /* maximum packet length */
#define QH_STS1_C      (1 << 27) /* control endpoint flag */
#define QH_STS1_RL     (1 << 28) /* nak count reload */

#define QH_STS2_ISM(n) ((n) << 0) /* interrupt schedule mask */
#define QH_STS2_SCM(n) ((n) << 8) /* split completion mask */
#define QH_STS2_HUB(n) ((n) << 16) /* hub address */
#define QH_STS2_PN(n)  ((n) << 23) /* port number */
#define QH_STS2_MUL(n) ((n) << 30) /* high-bandwidth pipe multiplier */

#define QH_BUF0_NAK(n) ((n) << 1) /* nak counter */
#define QH_BUF1_PS     (1 << 0) /* ping state */
#define QH_BUF1_ERR(n) ((n) << 10) /* error counter */
#define QH_BUF1_IOC    (1 << 15) /* interrupt on completion */
#define QH_BUF1_DT     (1 << 31) /* data toggle */
#define QH_BUF3_SCP(n) ((n) << 0) /* split-transaction complete-split progress */
#define QH_BUF4_SFT(n) ((n) << 0) /* split-transaction frame tag */
#define QH_BUF4_SB(n)  ((n) << 5) /* S-bytes */

#define MAX_PORTS 15

struct ehci_intr_pipe;
struct ehci_isoc_pipe;
struct ehci_itd;
struct ehci_qtd;
struct ehci_qh;

TAILQ_HEAD(ehci_intr_pipe_head, ehci_intr_pipe);
TAILQ_HEAD(ehci_isoc_pipe_head, ehci_isoc_pipe);
TAILQ_HEAD(ehci_itd_head, ehci_itd);
TAILQ_HEAD(ehci_qtd_head, ehci_qtd);
TAILQ_HEAD(ehci_qh_head, ehci_qh);

struct ehci
{
	struct usb_hcd hcd;
	struct pci_device *device;
	struct pci_map *pci_map;
	struct irq_handle irq_handle;
	struct dma_buf *frame_list_dma;
	uint32_t *frame_list;
	struct dma_buf *itd_dma;
	struct ehci_itd *itd;
	struct ehci_itd_head free_itd;
	struct dma_buf *qtd_dma;
	struct ehci_qtd *qtd;
	struct ehci_qtd_head free_qtd;
	struct dma_buf *qh_dma;
	struct ehci_qh *qh;
	struct ehci_qh_head free_qh;
	struct ehci_qh_head async_qh;
	uint8_t device_addr;
	uint8_t reg_offset;
	struct waitq waitq;
	struct mutex mutex;
	struct ehci_intr_pipe_head intr_pipes;
	struct ehci_isoc_pipe_head isoc_pipes;
	struct usb_device *devices[MAX_PORTS];
};

struct ehci_itd
{
	uint32_t link;
	struct
	{
		uint16_t offset;
		uint16_t length;
	} transaction[8];
	uint32_t buffer[7];
	TAILQ_ENTRY(ehci_itd) chain;
	uint32_t paddr;
} __attribute__((aligned(32)));

struct ehci_qtd
{
	uint32_t next;
	uint32_t alt;
	uint16_t status;
	uint16_t length;
	uint32_t buffer[5];
	TAILQ_ENTRY(ehci_qtd) chain;
	uint32_t paddr;
	uint16_t input_length;
} __attribute__((aligned(32)));

struct ehci_qh
{
	uint32_t link;
	uint32_t sts1;
	uint32_t sts2;
	uint32_t curr;
	uint32_t next;
	uint32_t alt;
	uint16_t status;
	uint16_t length;
	uint32_t buffer[5];
	TAILQ_ENTRY(ehci_qh) chain;
	uint32_t paddr;
} __attribute__((aligned(32)));

struct ehci_intr_pipe
{
	struct usb_intr_pipe *pipe;
	TAILQ_ENTRY(ehci_intr_pipe) chain;
	TAILQ_ENTRY(ehci_intr_pipe) poll_chain;
	struct ehci_qh *qh;
	struct ehci_qtd_head qtd_list;
	ssize_t ret;
};

struct ehci_isoc_pipe
{
	struct usb_isoc_pipe *pipe;
	TAILQ_ENTRY(ehci_isoc_pipe) chain;
};

static inline uint32_t ehci_read(struct ehci *ehci, uint32_t reg)
{
	return pci_r32(ehci->pci_map, ehci->reg_offset + reg);
}

static inline void ehci_write(struct ehci *ehci, uint32_t reg, uint32_t val)
{
	pci_w32(ehci->pci_map, ehci->reg_offset + reg, val);
}

static inline void ehci_lock(struct ehci *ehci)
{
	mutex_lock(&ehci->mutex);
}

static inline void ehci_unlock(struct ehci *ehci)
{
	mutex_unlock(&ehci->mutex);
}

static struct ehci_itd *itd_alloc(struct ehci *ehci)
{
	struct ehci_itd *itd = TAILQ_LAST(&ehci->free_itd, ehci_itd_head);
	if (!itd)
		return NULL;
	TAILQ_REMOVE(&ehci->free_itd, itd, chain);
	return itd;
}

static void itd_free(struct ehci *ehci, struct ehci_itd *itd)
{
	if (!itd)
		return;
	TAILQ_INSERT_TAIL(&ehci->free_itd, itd, chain);
}

static struct ehci_qtd *qtd_alloc(struct ehci *ehci)
{
	struct ehci_qtd *qtd = TAILQ_LAST(&ehci->free_qtd, ehci_qtd_head);
	if (!qtd)
		return NULL;
	TAILQ_REMOVE(&ehci->free_qtd, qtd, chain);
	return qtd;
}

static void qtd_free(struct ehci *ehci, struct ehci_qtd *qtd)
{
	if (!qtd)
		return;
	TAILQ_INSERT_TAIL(&ehci->free_qtd, qtd, chain);
}

static void qtd_enqueue(struct ehci_qtd_head *qtd_list, struct ehci_qtd *qtd)
{
	if (!TAILQ_EMPTY(qtd_list))
		TAILQ_LAST(qtd_list, ehci_qtd_head)->next = qtd->paddr;
	TAILQ_INSERT_TAIL(qtd_list, qtd, chain);
}

static void qtd_setup(struct ehci_qtd *qtd, uint32_t pid, uint32_t datat,
                      uint32_t maxlen, uint32_t buffer)
{
	qtd->next = QTD_NEXT_T;
	qtd->alt = QTD_NEXT_T;
	qtd->status = QTD_STS_ACTIVE | QTD_STS_CERR(1);
	switch (pid)
	{
		case USB_PID_OUT:
			qtd->status |= QTD_STS_PID(0);
			break;
		case USB_PID_IN:
			qtd->status |= QTD_STS_PID(1);
			break;
		case USB_PID_SETUP:
			qtd->status |= QTD_STS_PID(2);
			break;
		default:
			panic("unknown pid\n");
	}
	qtd->length = maxlen | (datat ? QTD_LEN_DT : 0);
	qtd->input_length = maxlen;
	/* XXX assert buffer is not crossing page */
	qtd->buffer[0] = buffer;
	qtd->buffer[1] = 0;
	qtd->buffer[2] = 0;
	qtd->buffer[3] = 0;
	qtd->buffer[4] = 0;
}

static size_t qtd_list_count_bytes(struct ehci_qtd_head *qtd_list,
                                   int skip_first)
{
	struct ehci_qtd *qtd;
	size_t ret = 0;

	TAILQ_FOREACH(qtd, qtd_list, chain)
	{
		if (skip_first)
		{
			skip_first = 0;
			continue;
		}
		if (qtd->status & QTD_STS_ACTIVE)
			break;
		ret += qtd->input_length - (qtd->length & ~QTD_LEN_DT);
	}
	return ret;
}

static void qtd_list_free(struct ehci *ehci, struct ehci_qtd_head *qtd_list)
{
	struct ehci_qtd *qtd;

	while ((qtd = TAILQ_FIRST(qtd_list)))
	{
		TAILQ_REMOVE(qtd_list, qtd, chain);
		qtd_free(ehci, qtd);
	}
}

static struct ehci_qh *qh_alloc(struct ehci *ehci)
{
	struct ehci_qh *qh = TAILQ_LAST(&ehci->free_qh, ehci_qh_head);
	if (!qh)
		return NULL;
	TAILQ_REMOVE(&ehci->free_qh, qh, chain);
	return qh;
}

static void qh_free(struct ehci *ehci, struct ehci_qh *qh)
{
	if (!qh)
		return;
	TAILQ_INSERT_TAIL(&ehci->free_qh, qh, chain);
}

static void qh_setup(struct ehci_qh *qh, const struct usb_device *device,
                     const struct ehci_qtd_head *qtd_list)
{
	struct ehci_qtd *first = TAILQ_FIRST(qtd_list);

	qh->link = QH_LINK_T;
	qh->sts1 = QH_STS1_DA(device->addr)
	         | QH_STS1_EPS(device->speed)
	         | QH_STS1_DTC
	         | QH_STS1_MPL(device->desc.max_packet_size);
	if (device->speed != USB_SPEED_HIGH)
		qh->sts1 |= QH_STS1_C;
	qh->sts2 = QH_STS2_MUL(1);
	qh->curr = first->paddr;
	qh->next = qh->curr;
	qh->alt = QH_LINK_T;
	qh->length = first->length;
	qh->buffer[0] = first->buffer[0];
	qh->buffer[1] = first->buffer[1];
	qh->buffer[2] = first->buffer[2];
	qh->buffer[3] = first->buffer[3];
	qh->buffer[4] = first->buffer[4];
	__atomic_store_n(&qh->status, first->status, __ATOMIC_SEQ_CST); /* keep last as it contains the active bit */
}

static int qh_poll(struct ehci_qh *qh)
{
	uint16_t status = *(volatile uint16_t*)&qh->status;
	if (status & QTD_STS_ACTIVE)
		return -EAGAIN;
	if (status & (QTD_STS_TE | QTD_STS_DBE | QTD_STS_HALTED))
		return -EINVAL;
	/* XXX check more flags */
	return 0;
}

static int qh_wait(struct ehci *ehci, struct ehci_qh *qh)
{
	int ret;

	while (1)
	{
		ret = qh_poll(qh);
		if (ret != -EAGAIN)
			return ret;
		ret = waitq_wait_tail_mutex(&ehci->waitq, &ehci->mutex, NULL);
		if (ret)
			return ret;
	}
}

static void async_qh_enqueue(struct ehci *ehci, struct ehci_qh *qh)
{
	if (TAILQ_EMPTY(&ehci->async_qh))
	{
		/* XXX always add an empty qh? */
		TAILQ_INSERT_HEAD(&ehci->async_qh, qh, chain);
		qh->link = QH_LINK_QH | qh->paddr;
		qh->sts1 |= QH_STS1_H;
		ehci_write(ehci, REG_ASYNCLISTADDR, qh->paddr);
		ehci_write(ehci, REG_USBCMD, ehci_read(ehci, REG_USBCMD) | CMD_ASE);
		return;
	}
	struct ehci_qh *first = TAILQ_FIRST(&ehci->async_qh);
	struct ehci_qh *last = TAILQ_LAST(&ehci->async_qh, ehci_qh_head);
	qh->link = QH_LINK_QH | first->paddr;
	__atomic_store_n(&last->link, QH_LINK_QH | qh->paddr, __ATOMIC_SEQ_CST);
}

static void async_qh_dequeue(struct ehci *ehci, struct ehci_qh *qh)
{
	struct ehci_qh *first = TAILQ_FIRST(&ehci->async_qh);
	struct ehci_qh *last = TAILQ_LAST(&ehci->async_qh, ehci_qh_head);
	if (first == last)
	{
		if (first != qh)
			panic("ehci: corrupted async qh list\n");
		ehci_write(ehci, REG_USBCMD, ehci_read(ehci, REG_USBCMD) & ~CMD_ASE);
		TAILQ_REMOVE(&ehci->async_qh, qh, chain);
		return;
	}
	struct ehci_qh *prev = TAILQ_PREV(qh, ehci_qh_head, chain);
	struct ehci_qh *next = TAILQ_NEXT(qh, chain);
	if (!prev)
		prev = last;
	if (!next)
		next = first;
	prev->link = QH_LINK_QH | next->paddr;
	if (qh->sts1 & QH_STS1_H)
		prev->sts1 |= QH_STS1_H;
	TAILQ_REMOVE(&ehci->async_qh, qh, chain);
}

static void periodic_qh_enqueue(struct ehci *ehci, struct ehci_qh *qh)
{
	//
}

static void periodic_qh_dequeue(struct ehci *ehci, struct ehci_qh *qh)
{
	//
}

static int setup_data_qtd_list(struct ehci *ehci, struct ehci_qtd_head *qtd_list,
                               uint32_t pid, uint32_t datat, uint32_t data,
                               uint32_t size, size_t max_packet_size)
{
	struct ehci_qtd *qtd;
	size_t n;

	for (size_t i = 0; i < size; i += max_packet_size)
	{
		qtd = qtd_alloc(ehci);
		if (!qtd)
		{
			TRACE("ehci: qtd allocation failed");
			return -ENOMEM;
		}
		n = size - i;
		if (n > max_packet_size)
			n = max_packet_size;
		qtd_setup(qtd, pid, datat, n, data + i);
		qtd_enqueue(qtd_list, qtd);
		datat = !datat;
	}
	return 0;
}

static ssize_t ehci_ctrl_transfer(struct usb_hcd *hcd,
                                  struct usb_device *device,
                                  int in_out, uint32_t req,
                                  uint32_t data, size_t size)
{
	struct ehci_qtd_head qtd_list;
	struct ehci_qtd *qtd;
	struct ehci_qh *qh;
	struct ehci *ehci = hcd->userdata;
	ssize_t ret;

	if (!device->desc.max_packet_size)
		return -EINVAL;

	ehci_lock(ehci);
	TAILQ_INIT(&qtd_list);
	qh = qh_alloc(ehci);
	if (!qh)
	{
		TRACE("ehci: qh allocation failed");
		ret = -ENOMEM;
		goto end;
	}

	/* setup */
	qtd = qtd_alloc(ehci);
	if (!qtd)
	{
		TRACE("ehci: qtd allocation failed");
		ret = -ENOMEM;
		goto end;
	}
	qtd_setup(qtd, USB_PID_SETUP, 0, sizeof(struct usb_request), req);
	qtd_enqueue(&qtd_list, qtd);

	/* data */
	ret = setup_data_qtd_list(ehci, &qtd_list,
	                          in_out ? USB_PID_OUT : USB_PID_IN,
	                          1, data, size, device->desc.max_packet_size);
	if (ret)
		goto end;

	/* status */
	qtd = qtd_alloc(ehci);
	if (!qtd)
	{
		TRACE("ehci: qtd allocation failed");
		ret = -ENOMEM;
		goto end;
	}
	qtd_setup(qtd, in_out ? USB_PID_IN : USB_PID_OUT, 1, 0, 0);
	qtd->status |= QTD_STS_IOC;
	qtd_enqueue(&qtd_list, qtd);

	qh_setup(qh, device, &qtd_list);
	async_qh_enqueue(ehci, qh);
	ret = qh_wait(ehci, qh);
	async_qh_dequeue(ehci, qh);
	if (ret)
		goto end;
	ret = qtd_list_count_bytes(&qtd_list, 1);

end:
	qtd_list_free(ehci, &qtd_list);
	qh_free(ehci, qh);
	ehci_unlock(ehci);
	return ret;
}

static void ehci_intr_pipe_free(struct ehci *ehci, struct ehci_intr_pipe *pipe)
{
	if (!pipe)
		return;
	qtd_list_free(ehci, &pipe->qtd_list);
	qh_free(ehci, pipe->qh);
	free(pipe);
}

static int ehci_intr_transfer(struct usb_intr_pipe *usb_pipe)
{
	struct usb_endpoint *endpoint = usb_pipe->endpoint;
	struct usb_device *device = usb_pipe->device;
	struct usb_hcd *hcd = device->hcd;
	struct ehci *ehci = hcd->userdata;
	struct ehci_intr_pipe *pipe = NULL;
	int ret;

	if (!usb_pipe->buf->size)
	{
		TRACE("ehci: zero intr buf size");
		return -EINVAL;
	}
	if (!endpoint->desc.max_packet_size)
	{
		TRACE("ehci: zero intr max packet size");
		return -EINVAL;
	}
	pipe = malloc(sizeof(*pipe), M_ZERO);
	if (!pipe)
	{
		TRACE("ehci: intr pipe allocation failed");
		return -ENOMEM;
	}
	ehci_lock(ehci);
	TAILQ_INIT(&pipe->qtd_list);
	pipe->pipe = usb_pipe;
	pipe->qh = qh_alloc(ehci);
	if (!pipe->qh)
	{
		TRACE("ehci: qh allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	ret = setup_data_qtd_list(ehci, &pipe->qtd_list,
	                          USB_PID_IN, 0,
	                          pm_page_addr(usb_pipe->buf->pages),
	                          usb_pipe->buf->size,
	                          endpoint->desc.max_packet_size);
	if (ret)
		goto err;
	TAILQ_LAST(&pipe->qtd_list, ehci_qtd_head)->status |= QTD_STS_IOC;
	qh_setup(pipe->qh, device, &pipe->qtd_list);
	periodic_qh_enqueue(ehci, pipe->qh);
	TAILQ_INSERT_TAIL(&ehci->intr_pipes, pipe, chain);
	ehci_unlock(ehci);
	return 0;

err:
	ehci_intr_pipe_free(ehci, pipe);
	ehci_unlock(ehci);
	return ret;
}

static int ehci_isoc_transfer(struct usb_isoc_pipe *usb_pipe)
{
	(void)usb_pipe;
	/* XXX */
	return -EINVAL;
}

static int ehci_get_addr(struct usb_hcd *hcd, uint8_t *addr)
{
	struct ehci *ehci = hcd->userdata;

	ehci_lock(ehci);
	if (ehci->device_addr == 0x7F)
		return -EINVAL;
	*addr = ++ehci->device_addr;
	ehci_unlock(ehci);
	return 0;
}

static void setup_port(struct ehci *ehci, uint8_t port)
{
	static const struct timespec rst_delay = {0, 100000000};
	static const struct timespec pe_delay = {0, 1000000};
	uint32_t status;
	int ret;

	status = ehci_read(ehci, REG_PORTSC(port));
	if (!(status & PORTSC_CCS))
		return;
	ehci_write(ehci, REG_PORTSC(port), PORTSC_PR);
	spinsleep(&rst_delay);
	ehci_write(ehci, REG_PORTSC(port), 0);
	for (size_t i = 0;; ++i)
	{
		if (i == 100)
			return;
		status = ehci_read(ehci, REG_PORTSC(port));
		if (!(status & PORTSC_CCS))
			return;
		if (status & PORTSC_PE)
			break;
		spinsleep(&pe_delay);
	}
	ret = usb_device_alloc(&ehci->hcd, USB_SPEED_HIGH, &ehci->devices[port]);
	if (ret)
	{
		TRACE("ehci: failed to create device");
		return;
	}
	usb_device_probe(ehci->devices[port]);
}

static int setup_frame_list(struct ehci *ehci)
{
	struct ehci_itd *itd;
	struct ehci_qtd *qtd;
	struct ehci_qh *qh;
	int ret;

	ret = dma_buf_alloc(PAGE_SIZE, DMA_32BIT, &ehci->frame_list_dma);
	if (ret)
	{
		TRACE("ehci: frame list allocation failed");
		return ret;
	}
	ehci->frame_list = ehci->frame_list_dma->data;
	ret = dma_buf_alloc(PAGE_SIZE * 8, DMA_32BIT, &ehci->itd_dma);
	if (ret)
	{
		TRACE("ehci: itd allocation failed");
		return ret;
	}
	ehci->itd = ehci->itd_dma->data;
	for (size_t i = 0; i < ehci->itd_dma->size / sizeof(*ehci->itd); ++i)
	{
		itd = &ehci->itd[i];
		itd->paddr = pm_page_addr(ehci->itd_dma->pages) + i * sizeof(*ehci->itd);
		TAILQ_INSERT_TAIL(&ehci->free_itd, itd, chain);
	}
	ret = dma_buf_alloc(PAGE_SIZE * 8, DMA_32BIT, &ehci->qtd_dma);
	if (ret)
	{
		TRACE("ehci: qtd allocation failed");
		return ret;
	}
	ehci->qtd = ehci->qtd_dma->data;
	for (size_t i = 0; i < ehci->qtd_dma->size / sizeof(*ehci->qtd); ++i)
	{
		qtd = &ehci->qtd[i];
		qtd->paddr = pm_page_addr(ehci->qtd_dma->pages) + i * sizeof(*ehci->qtd);
		TAILQ_INSERT_TAIL(&ehci->free_qtd, qtd, chain);
	}
	ret = dma_buf_alloc(PAGE_SIZE, DMA_32BIT, &ehci->qh_dma);
	if (ret)
	{
		TRACE("ehci: qh allocation failed");
		return ret;
	}
	ehci->qh = ehci->qh_dma->data;
	for (size_t i = 0; i < ehci->qh_dma->size / sizeof(*ehci->qh); ++i)
	{
		qh = &ehci->qh[i];
		qh->paddr = pm_page_addr(ehci->qh_dma->pages) + i * sizeof(*ehci->qh);
		TAILQ_INSERT_TAIL(&ehci->free_qh, qh, chain);
	}
	for (size_t i = 0; i < PAGE_SIZE / sizeof(*ehci->frame_list); ++i)
		ehci->frame_list[i] = (1 << 0);
	ehci_write(ehci, REG_PERIODICLISTBASE, pm_page_addr(ehci->frame_list_dma->pages));
	return 0;
}

static const struct usb_hcd_op op =
{
	.get_addr = ehci_get_addr,
	.ctrl_transfer = ehci_ctrl_transfer,
	.intr_transfer = ehci_intr_transfer,
	.isoc_transfer = ehci_isoc_transfer,
};

static void poll_intr_pipes(struct ehci *ehci)
{
	struct ehci_intr_pipe_head completed;
	struct ehci_intr_pipe *pipe;
	struct ehci_qtd *qtd;
	int ret;

	TAILQ_INIT(&completed);
	TAILQ_FOREACH(pipe, &ehci->intr_pipes, chain)
	{
		ret = qh_poll(pipe->qh);
		if (ret == -EAGAIN)
			continue;
		if (!ret)
			ret = qtd_list_count_bytes(&pipe->qtd_list, 0);
		pipe->ret = ret;
		TAILQ_INSERT_TAIL(&completed, pipe, poll_chain);
	}
	ehci_unlock(ehci);
	while ((pipe = TAILQ_FIRST(&completed)))
	{
		TAILQ_REMOVE(&completed, pipe, poll_chain);
		pipe->pipe->fn(pipe->pipe, pipe->ret);
		TAILQ_FOREACH(qtd, &pipe->qtd_list, chain)
		{
			/* XXX re-setup */
		}
		TAILQ_LAST(&pipe->qtd_list, ehci_qtd_head)->status |= QTD_STS_IOC;
		qh_setup(pipe->qh, pipe->pipe->device, &pipe->qtd_list);
	}
	ehci_lock(ehci);
}

static void poll_isoc_pipes(struct ehci *ehci)
{
}

static void int_handler(void *userptr)
{
	struct ehci *ehci = userptr;
	uint32_t status = ehci_read(ehci, REG_USBSTS);

	if (!(status & 0x3F))
		return;
	/* it's not the correct way to do this, but not asserting every
	 * interrupt make the interrupt mechanism blocked
	 */
	ehci_write(ehci, REG_USBSTS, 0x3F);
	ehci_lock(ehci);
	poll_intr_pipes(ehci);
	poll_isoc_pipes(ehci);
	waitq_broadcast(&ehci->waitq, 0);
	ehci_unlock(ehci);
}

void ehci_free(struct ehci *ehci)
{
	if (!ehci)
		return;
	dma_buf_free(ehci->itd_dma);
	dma_buf_free(ehci->qtd_dma);
	dma_buf_free(ehci->qh_dma);
	dma_buf_free(ehci->frame_list_dma);
	mutex_destroy(&ehci->mutex);
	waitq_destroy(&ehci->waitq);
	pci_unmap(ehci->device, ehci->pci_map);
	pci_unregister_irq(ehci->device, &ehci->irq_handle);
	/* XXX intr pipes */
	/* XXX isoc pipes */
	free(ehci);
}

int init_pci(struct pci_device *device, void *userdata)
{
	struct ehci *ehci = NULL;
	int ret;

	(void)userdata;
	ehci = malloc(sizeof(*ehci), M_ZERO);
	if (!ehci)
	{
		TRACE("ehci: ehci allocation failed");
		return -ENOMEM;
	}
	ehci->hcd.op = &op;
	ehci->hcd.userdata = ehci;
	TAILQ_INIT(&ehci->free_itd);
	TAILQ_INIT(&ehci->free_qtd);
	TAILQ_INIT(&ehci->free_qh);
	TAILQ_INIT(&ehci->async_qh);
	TAILQ_INIT(&ehci->intr_pipes);
	TAILQ_INIT(&ehci->isoc_pipes);
	waitq_init(&ehci->waitq);
	mutex_init(&ehci->mutex, 0);
	ehci->device = device;
	ret = pci_map_bar(device, 0, PAGE_SIZE, 0, &ehci->pci_map);
	if (ret)
	{
		TRACE("ehci: failed to init bar0");
		goto err;
	}
	ehci->reg_offset = pci_r8(ehci->pci_map, 0);
	ehci_write(ehci, REG_USBCMD, CMD_HCRESET);
	while (ehci_read(ehci, REG_USBCMD) & CMD_HCRESET) /* XXX timeout */
		;
	ret = setup_frame_list(ehci);
	if (ret)
		goto err;
	ret = pci_register_irq(device, int_handler, ehci, &ehci->irq_handle);
	if (ret)
	{
		TRACE("ehci: failed to enable irq");
		goto err;
	}
	ehci_write(ehci, REG_USBINTR, 0x7);
	ehci_write(ehci, REG_USBCMD, CMD_RS | CMD_FLS(0) | CMD_ITC(0x8));
	for (size_t i = 0; i < (pci_r32(ehci->pci_map, 0x4) & 0xF); ++i)
		setup_port(ehci, i);
	return 0;

err:
	ehci_free(ehci);
	return ret;
}

int init(void)
{
	pci_probe_progif(PCI_CLASS_SERIAL_BUS,
	                 PCI_SUBCLASS_SERIAL_BUS_USB,
	                 PCI_PROGIF_SERIAL_BUS_USB_EHCI,
	                 init_pci, NULL);
	return 0;
}

void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "usb/ehci",
	.init = init,
	.fini = fini,
};
