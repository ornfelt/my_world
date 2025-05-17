#define ENABLE_TRACE

#include "usb.h"
#include "pci.h"

#include <errno.h>
#include <queue.h>
#include <kmod.h>
#include <std.h>
#include <mem.h>

/* Universal Host Controller Interface (UHCI) Design Guide
 * Revision 1.1
 */

#define REG_USBCMD    (0x00) /* USB command */
#define REG_USBSTS    (0x02) /* USB status */
#define REG_USBINTR   (0x04) /* USB interrupt enable */
#define REG_FRNUM     (0x06) /* frame number */
#define REG_FRBASEADD (0x08) /* frame list base */
#define REG_SOFMOD    (0x0C) /* start of frame modify */
#define REG_PORTSC(n) (0x10 + 0x2 * (n)) /* port 1 status / control */

#define CMD_RS      (1 << 0) /* run / stop */
#define CMD_HCRESET (1 << 1) /* host controller reset */
#define CMD_GRESET  (1 << 2) /* global reset */
#define CMD_EGSM    (1 << 3) /* enter global suspend mode */
#define CMD_FGR     (1 << 4) /* force global resume */
#define CMD_SWDBG   (1 << 5) /* software debug */
#define CMD_CF      (1 << 6) /* configure flag */
#define CMD_MAXP    (1 << 7) /* max packet */

#define PORTSC_CCS  (1 << 0) /* current connect status */
#define PORTSC_CSC  (1 << 1) /* connect status change */
#define PORTSC_PE   (1 << 2) /* port enable / disable */
#define PORTSC_PEC  (1 << 3) /* port enable / disable change */
#define PORTSC_LS   (3 << 4) /* line status */
#define PORTSC_RD   (1 << 6) /* resume detect */
#define PORTSC_LSDA (1 << 8) /* low speed device attached */
#define PORTSC_RST  (1 << 9) /* port reset */
#define PORTSC_SUS  (1 << 12) /* suspend */

#define TD_FLAG_BSE     (1 << 1) /* bitstuff error */
#define TD_FLAG_CRC     (1 << 2) /* CRC / timeout error */
#define TD_FLAG_NAK     (1 << 3) /* nak received */
#define TD_FLAG_BABBLE  (1 << 4) /* babble detected */
#define TD_FLAG_DBE     (1 << 5) /* data buffer error */
#define TD_FLAG_STALLED (1 << 6) /* stalled */
#define TD_FLAG_ACTIVE  (1 << 7) /* active */
#define TD_FLAG_IOC     (1 << 8) /* interrupt on completion */
#define TD_FLAG_IOS     (1 << 9) /* isochronous select */
#define TD_FLAG_LS      (1 << 10) /* low speed */
#define TD_FLAG_CTR(n)  ((n) << 11) /* error counter */
#define TD_FLAG_SPD     (1 << 13) /* short packet detected */

#define TD_VF (1 << 2) /* depth / breath select */
#define TD_Q  (1 << 1) /* QH/TD select */
#define TD_T  (1 << 0) /* terminate */

#define QH_Q (1 << 1) /* QH/TD select */
#define QH_T (1 << 0) /* terminate */

#define MAX_PORTS 8

#define ISOC_MAX_TD 16

struct uhci_intr_pipe;
struct uhci_isoc_pipe;
struct uhci_qh;
struct uhci_td;

TAILQ_HEAD(uhci_intr_pipe_head, uhci_intr_pipe);
TAILQ_HEAD(uhci_isoc_pipe_head, uhci_isoc_pipe);
TAILQ_HEAD(uhci_qh_head, uhci_qh);
TAILQ_HEAD(uhci_td_head, uhci_td);

struct uhci
{
	struct usb_hcd hcd;
	struct pci_device *device;
	struct pci_map *pci_map;
	struct irq_handle irq_handle;
	struct dma_buf *frame_list_dma;
	uint32_t *frame_list;
	struct dma_buf *td_dma;
	struct uhci_td *td;
	struct uhci_td_head free_td;
	struct dma_buf *qh_dma;
	struct uhci_qh *qh;
	struct uhci_qh *root_qh;
	struct uhci_qh *intr_qh;
	struct uhci_qh *ctrl_qh;
	struct uhci_qh *bulk_qh;
	struct uhci_qh_head free_qh;
	uint8_t device_addr;
	struct waitq waitq;
	struct mutex mutex;
	struct uhci_intr_pipe_head intr_pipes;
	struct uhci_isoc_pipe_head isoc_pipes;
	struct usb_device *devices[MAX_PORTS];
};

struct uhci_td
{
	uint32_t link;
	uint16_t length;
	uint16_t flags;
	uint32_t token;
	uint32_t buffer;
	TAILQ_ENTRY(uhci_td) chain;
} __attribute__((aligned(32)));

struct uhci_qh
{
	uint32_t link;
	uint32_t elem;
	TAILQ_ENTRY(uhci_qh) chain;
} __attribute__((aligned(16)));

struct uhci_intr_pipe
{
	struct usb_intr_pipe *pipe;
	TAILQ_ENTRY(uhci_intr_pipe) chain;
	TAILQ_ENTRY(uhci_intr_pipe) poll_chain;
	struct uhci_qh *qh;
	struct uhci_td_head td_list;
	ssize_t ret;
};

struct uhci_isoc_pipe
{
	struct usb_isoc_pipe *pipe;
	TAILQ_ENTRY(uhci_isoc_pipe) chain;
	TAILQ_ENTRY(uhci_isoc_pipe) poll_chain;
	struct uhci_td_head td_lists[2];
	size_t td_pos[2]; /* td_list position in frame pointer list */
	size_t td_list_size; /* number of bytes for each td_list */
	size_t td_count; /* total number of td for the pipe */
	int td_ff; /* which list is currently being processed */
};

static inline uint16_t
uhci_r16(struct uhci *uhci, uint32_t reg)
{
	return pci_r16(uhci->pci_map, reg);
}

static inline uint32_t
uhci_r32(struct uhci *uhci, uint32_t reg)
{
	return pci_r32(uhci->pci_map, reg);
}

static inline void
uhci_w16(struct uhci *uhci, uint32_t reg, uint16_t val)
{
	pci_w16(uhci->pci_map, reg, val);
}

static inline void
uhci_w32(struct uhci *uhci, uint32_t reg, uint32_t val)
{
	pci_w32(uhci->pci_map, reg, val);
}

static inline void
uhci_lock(struct uhci *uhci)
{
	mutex_lock(&uhci->mutex);
}

static inline void
uhci_unlock(struct uhci *uhci)
{
	mutex_unlock(&uhci->mutex);
}

static struct uhci_td *
td_alloc(struct uhci *uhci)
{
	struct uhci_td *td = TAILQ_LAST(&uhci->free_td, uhci_td_head);
	if (!td)
		return NULL;
	TAILQ_REMOVE(&uhci->free_td, td, chain);
	return td;
}

static void
td_free(struct uhci *uhci, struct uhci_td *td)
{
	if (!td)
		return;
	TAILQ_INSERT_TAIL(&uhci->free_td, td, chain);
}

static uint32_t
td_paddr(struct uhci *uhci, struct uhci_td *td)
{
	return pm_page_addr(uhci->td_dma->pages) + ((uint8_t*)td - (uint8_t*)uhci->td);
}

static struct uhci_td *
td_from_paddr(struct uhci *uhci, uint32_t paddr)
{
	uintptr_t base = pm_page_addr(uhci->td_dma->pages);
	if (paddr % sizeof(uhci->td)
	 || paddr < base
	 || paddr >= base + uhci->td_dma->size)
		return NULL;
	return &uhci->td[(paddr - base) / sizeof(*uhci->td)];
}

static void
td_enqueue(struct uhci *uhci, struct uhci_td_head *list, struct uhci_td *td)
{
	if (!TAILQ_EMPTY(list))
		TAILQ_LAST(list, uhci_td_head)->link = TD_VF | td_paddr(uhci, td);
	TAILQ_INSERT_TAIL(list, td, chain);
}

static void
td_setup(struct uhci_td *td,
         uint32_t pid,
         uint32_t dev,
         uint32_t endp,
         uint32_t datat,
         uint32_t maxlen,
         uint32_t buffer)
{
	maxlen = (maxlen - 1) & 0x7FF;
	td->link = TD_T;
	td->length = 0x7FF;
	td->flags = TD_FLAG_ACTIVE | TD_FLAG_CTR(1);
	td->token = ((pid & 0xFF) << 0)
	          | ((dev & 0x7F) << 8)
	          | ((endp & 0xF) << 15)
	          | ((datat & 0x1) << 19)
	          | (maxlen << 21);
	td->buffer = buffer;
}

static size_t
td_list_count_bytes(struct uhci_td_head *td_list, int skip_first)
{
	struct uhci_td *td;
	size_t ret = 0;

	TAILQ_FOREACH(td, td_list, chain)
	{
		if (skip_first)
		{
			skip_first = 0;
			continue;
		}
		if (td->flags & TD_FLAG_ACTIVE)
			break;
		ret += (td->length + 1) & 0x7FF;
	}
	return ret;
}

static void
td_list_free(struct uhci *uhci, struct uhci_td_head *td_list)
{
	struct uhci_td *td;

	while ((td = TAILQ_FIRST(td_list)))
	{
		TAILQ_REMOVE(td_list, td, chain);
		td_free(uhci, td);
	}
}

static struct uhci_qh *
qh_alloc(struct uhci *uhci)
{
	struct uhci_qh *qh;

	qh = TAILQ_LAST(&uhci->free_qh, uhci_qh_head);
	if (!qh)
		return NULL;
	TAILQ_REMOVE(&uhci->free_qh, qh, chain);
	return qh;
}

static void
qh_free(struct uhci *uhci, struct uhci_qh *qh)
{
	if (!qh)
		return;
	TAILQ_INSERT_TAIL(&uhci->free_qh, qh, chain);
}

static uint32_t
qh_paddr(struct uhci *uhci, struct uhci_qh *qh)
{
	return pm_page_addr(uhci->qh_dma->pages) + ((uint8_t*)qh - (uint8_t*)uhci->qh);
}

static struct uhci_qh *
qh_from_paddr(struct uhci *uhci, uint32_t paddr)
{
	uintptr_t base = pm_page_addr(uhci->qh_dma->pages);
	if (paddr % sizeof(uhci->qh)
	 || paddr < base
	 || paddr >= base + uhci->qh_dma->size)
		return NULL;
	return &uhci->qh[(paddr - base) / sizeof(*uhci->qh)];
}

static void
qh_remove(struct uhci *uhci, struct uhci_qh *head, struct uhci_qh *qh)
{
	struct uhci_qh *prev = NULL;
	struct uhci_qh *it;
	uint32_t elem;

	elem = head->elem;
	while (1)
	{
		if (!(elem & QH_Q) || (elem & QH_T))
		{
			TRACE("uhci: qh not found");
			return;
		}
		it = qh_from_paddr(uhci, elem & ~0xF);
		if (!it)
		{
			TRACE("uhci: invalid qh");
			return;
		}
		if (it != qh)
		{
			elem = it->link;
			prev = it;
			continue;
		}
		/* hopefully, there's no hardware race condition in there */
		if (prev)
			prev->link = qh->link;
		else
			head->elem = qh->link;
		return;
	}
}

/* XXX
 * push to front is not the best thing to do, but it's really
 * helpful because it's way easier to handle
 */
static void
qh_queue(struct uhci *uhci, struct uhci_qh *head, struct uhci_qh *qh)
{
	qh->link = head->elem;
	head->elem = QH_Q | qh_paddr(uhci, qh);
	__atomic_thread_fence(__ATOMIC_RELEASE);
}

static int
qh_poll(struct uhci *uhci, struct uhci_qh *qh, struct uhci_td_head *td_list)
{
	uint32_t td_addr = *(volatile uint32_t*)&qh->elem & ~0xF;
	if (!td_addr)
		return 0;
	struct uhci_td *td = td_from_paddr(uhci, td_addr);
	if (!td)
	{
		TRACE("uhci: bogus td");
		return -EINVAL;
	}
	uint16_t flags = *(volatile uint16_t*)&td->flags;
	if (flags & TD_FLAG_ACTIVE)
		return -EAGAIN;
	if (flags & TD_FLAG_STALLED)
		return -EINVAL;
	/* XXX maybe there's other conditions to detect
	 * end of processing without race condition
	 */
	if (td != TAILQ_LAST(td_list, uhci_td_head))
		return -EAGAIN;
	return 0;
}

static int
qh_wait(struct uhci *uhci, struct uhci_qh *qh, struct uhci_td_head *td_list)
{
	int ret;

	while (1)
	{
		ret = qh_poll(uhci, qh, td_list);
		if (ret != -EAGAIN)
			return ret;
		ret = waitq_wait_tail_mutex(&uhci->waitq, &uhci->mutex, NULL);
		if (ret)
			return ret;
	}
}

static int
setup_data_td_list(struct uhci *uhci,
                   struct uhci_td_head *td_list,
                   uint32_t pid,
                   uint32_t dev,
                   uint32_t endp,
                   uint32_t datat,
                   uint32_t data,
                   size_t size,
                   size_t max_packet_size)
{
	struct uhci_td *td;
	size_t n;

	for (size_t i = 0; i < size; i += max_packet_size)
	{
		td = td_alloc(uhci);
		if (!td)
		{
			TRACE("uhci: td allocation failed");
			return -ENOMEM;
		}
		n = size - i;
		if (n > max_packet_size)
			n = max_packet_size;
		td_setup(td, pid, dev, endp, datat, n, data + i);
		td_enqueue(uhci, td_list, td);
		datat = !datat;
	}
	return 0;
}

static ssize_t
uhci_ctrl_transfer(struct usb_hcd *hcd,
                   struct usb_device *device,
                   int in_out,
                   uint32_t req,
                   uint32_t data,
                   size_t size)
{
	struct uhci_td_head td_list;
	struct uhci_qh *qh;
	struct uhci_td *td;
	struct uhci *uhci = hcd->userdata;
	ssize_t ret;

	if (!device->desc.max_packet_size)
		return -EINVAL;

	uhci_lock(uhci);
	TAILQ_INIT(&td_list);
	qh = qh_alloc(uhci);
	if (!qh)
	{
		TRACE("uhci: qh allocation failed");
		ret = -ENOMEM;
		goto end;
	}

	/* setup */
	td = td_alloc(uhci);
	if (!td)
	{
		TRACE("uhci: td allocation failed");
		ret = -ENOMEM;
		goto end;
	}
	td_setup(td, USB_PID_SETUP, device->addr, 0, 0, sizeof(struct usb_request), req);
	td_enqueue(uhci, &td_list, td);

	/* data */
	ret = setup_data_td_list(uhci, &td_list,
	                         in_out ? USB_PID_OUT : USB_PID_IN,
	                         device->addr, 0, 1, data, size,
	                         device->desc.max_packet_size);
	if (ret)
		goto end;

	/* status */
	td = td_alloc(uhci);
	if (!td)
	{
		TRACE("uhci: td allocation failed");
		ret = -ENOMEM;
		goto end;
	}
	td_setup(td, in_out ? USB_PID_IN : USB_PID_OUT, device->addr, 0, 1, 0, 0);
	td->flags |= TD_FLAG_IOC;
	td_enqueue(uhci, &td_list, td);

	qh->link = QH_T;
	qh->elem = td_paddr(uhci, TAILQ_FIRST(&td_list));

	qh_queue(uhci, uhci->ctrl_qh, qh);
	ret = qh_wait(uhci, qh, &td_list);
	qh_remove(uhci, uhci->ctrl_qh, qh);
	if (ret)
		goto end;
	ret = td_list_count_bytes(&td_list, 1);

end:
	td_list_free(uhci, &td_list);
	qh_free(uhci, qh);
	uhci_unlock(uhci);
	return ret;
}

static void
uhci_intr_pipe_free(struct uhci *uhci, struct uhci_intr_pipe *pipe)
{
	if (!pipe)
		return;
	td_list_free(uhci, &pipe->td_list);
	qh_free(uhci, pipe->qh);
	free(pipe);
}

static int
uhci_intr_transfer(struct usb_intr_pipe *usb_pipe)
{
	struct usb_endpoint *endpoint = usb_pipe->endpoint;
	struct usb_device *device = usb_pipe->device;
	struct usb_hcd *hcd = device->hcd;
	struct uhci *uhci = hcd->userdata;
	struct uhci_intr_pipe *pipe = NULL;
	int ret;

	if (!usb_pipe->buf->size)
	{
		TRACE("uhci: zero intr buf size");
		return -EINVAL;
	}
	if (!endpoint->desc.max_packet_size)
	{
		TRACE("uhci: zero intr max packet size");
		return -EINVAL;
	}
	pipe = malloc(sizeof(*pipe), M_ZERO);
	if (!pipe)
	{
		TRACE("uhci: intr pipe allocation failed");
		return -ENOMEM;
	}
	uhci_lock(uhci);
	TAILQ_INIT(&pipe->td_list);
	pipe->pipe = usb_pipe;
	pipe->qh = qh_alloc(uhci);
	if (!pipe->qh)
	{
		TRACE("uhci: qh allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	ret = setup_data_td_list(uhci, &pipe->td_list,
	                         USB_PID_IN, device->addr,
	                         endpoint->desc.address,
	                         0, pm_page_addr(usb_pipe->buf->pages),
	                         usb_pipe->buf->size,
	                         endpoint->desc.max_packet_size);
	if (ret)
		goto err;
	TAILQ_LAST(&pipe->td_list, uhci_td_head)->flags |= TD_FLAG_IOC;
	pipe->qh->link = QH_T;
	pipe->qh->elem = td_paddr(uhci, TAILQ_FIRST(&pipe->td_list));
	qh_queue(uhci, uhci->intr_qh, pipe->qh);
	TAILQ_INSERT_TAIL(&uhci->intr_pipes, pipe, chain);
	uhci_unlock(uhci);
	return 0;

err:
	uhci_intr_pipe_free(uhci, pipe);
	uhci_unlock(uhci);
	return ret;
}

static void
uhci_isoc_pipe_free(struct uhci *uhci, struct uhci_isoc_pipe *pipe)
{
	if (!pipe)
		return;
	/* XXX remove td from frame pointers */
	td_list_free(uhci, &pipe->td_lists[0]);
	td_list_free(uhci, &pipe->td_lists[1]);
	free(pipe);
}

static uint32_t
npot32(uint32_t val)
{
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	return ++val;
}

static void
insert_isoc_queue(struct uhci *uhci, struct uhci_td_head *td_list, size_t frame)
{
	struct uhci_td *td;

	TAILQ_FOREACH(td, td_list, chain)
	{
		td->link = uhci->frame_list[frame];
		__atomic_store_n(&uhci->frame_list[frame],
		                 td_paddr(uhci, td),
		                 __ATOMIC_RELEASE);
		frame = (frame + 1) % 1024;
	}
}

/* XXX
 * this is really overcomplicated.
 * maybe we should mirror the td / qh links in the uhci_td / uhci_qh structs
 * to make everything easier ?
 */
static void
remove_isoc_queue(struct uhci *uhci,
                  struct uhci_td_head *td_list,
                  size_t frame)
{
	struct uhci_td *td;

	TAILQ_FOREACH(td, td_list, chain)
	{
		struct uhci_td *prev = NULL;
		struct uhci_td *it;
		uint32_t elem = uhci->frame_list[frame];
		while (1)
		{
			if ((elem & (QH_Q | QH_T)))
			{
				TRACE("uhci: td not found");
				return; /* XXX */
			}
			it = td_from_paddr(uhci, elem & ~0xF);
			if (!it)
			{
				TRACE("uhci: invalid td");
				return; /* XXX */
			}
			if (it != td)
			{
				elem = it->link;
				prev = it;
				continue;
			}
			if (prev)
				prev->link = td->link;
			else
				__atomic_store_n(&uhci->frame_list[frame], td->link, __ATOMIC_RELEASE);
			break;
		}
		frame = (frame + 1) % 1024;
	}
}

static void
refill_isoc_pipe(struct uhci_isoc_pipe *pipe, int ff)
{
	struct usb_endpoint *endpoint = pipe->pipe->endpoint;
	size_t sizes[ISOC_MAX_TD];
	uint8_t *data;

	for (size_t i = 0; i < pipe->td_count / 2; ++i)
		sizes[i] = endpoint->desc.max_packet_size;
	data = pipe->pipe->buf->data;
	if (ff)
		data += pipe->td_list_size;
	pipe->pipe->fn(pipe->pipe, data, sizes, pipe->td_count / 2);
}

static int
uhci_isoc_transfer(struct usb_isoc_pipe *usb_pipe)
{
	struct usb_endpoint *endpoint = usb_pipe->endpoint;
	struct usb_device *device = usb_pipe->device;
	struct usb_hcd *hcd = device->hcd;
	struct uhci *uhci = hcd->userdata;
	uintptr_t data = pm_page_addr(usb_pipe->buf->pages);
	struct uhci_isoc_pipe *pipe = NULL;
	size_t pkt_size = endpoint->desc.max_packet_size;
	struct uhci_td *td;
	size_t td_list_size;
	size_t td_count;
	int ret;

	if (!pkt_size)
		return -EINVAL;
	td_count = PAGE_SIZE / pkt_size;
	if (td_count < 2)
	{
		TRACE("uhci: isoc packet size too big");
		return -EINVAL;
	}
	/* don't bother allocating too much td, limit buffer latency
	 */
	if (td_count > ISOC_MAX_TD)
		td_count = ISOC_MAX_TD;
	/* make td_count a power of two, rounding down
	 * power of two is required because it allows insertion of td
	 * inside the frame pointers without any remainder
	 * (because frame pointers list is also a power of two)
	 */
	if (td_count & (td_count - 1))
		td_count = npot32(td_count) >> 1;
	td_list_size = td_count * pkt_size / 2;
	pipe = malloc(sizeof(*pipe), M_ZERO);
	if (!pipe)
	{
		TRACE("uhci: isoc pipe allocation failed");
		return -ENOMEM;
	}
	uhci_lock(uhci);
	pipe->td_count = td_count;
	pipe->td_list_size = td_list_size;
	pipe->pipe = usb_pipe;
	for (size_t i = 0; i < 2; ++i)
	{
		TAILQ_INIT(&pipe->td_lists[i]);
		ret = setup_data_td_list(uhci, &pipe->td_lists[i],
		                         USB_PID_OUT, device->addr,
		                         endpoint->desc.address,
		                         0, data + (i ? td_list_size : 0),
		                         td_list_size, pkt_size);
		if (ret)
			goto err;
		TAILQ_FOREACH(td, &pipe->td_lists[i], chain)
		{
			td->flags |= TD_FLAG_IOS;
			td->token &= ~(1 << 19); /* must be data0 */
		}
		TAILQ_LAST(&pipe->td_lists[i], uhci_td_head)->flags |= TD_FLAG_IOC;
	}
	td = TAILQ_LAST(&pipe->td_lists[0], uhci_td_head);
	pipe->td_pos[0] = uhci_r16(uhci, REG_FRNUM) + 2 + td_list_size;
	/* align position on td_list_size (which is a power of two)
	 * to make all the isochronous transfers aligned on power of two
	 * this reduces the number of IOC interrupts
	 */
	pipe->td_pos[0] -= pipe->td_pos[0] % td_list_size;
	pipe->td_pos[0] %= 1024;
	refill_isoc_pipe(pipe, 0);
	insert_isoc_queue(uhci, &pipe->td_lists[0], pipe->td_pos[0]);
	pipe->td_pos[1] = (pipe->td_pos[0] + pipe->td_count / 2) % 1024;
	refill_isoc_pipe(pipe, 1);
	insert_isoc_queue(uhci, &pipe->td_lists[1], pipe->td_pos[1]);
	TAILQ_INSERT_TAIL(&uhci->isoc_pipes, pipe, chain);
	uhci_unlock(uhci);
	return 0;

err:
	uhci_isoc_pipe_free(uhci, pipe);
	uhci_unlock(uhci);
	return ret;
}

static int
uhci_get_addr(struct usb_hcd *hcd, uint8_t *addr)
{
	struct uhci *uhci = hcd->userdata;

	uhci_lock(uhci);
	if (uhci->device_addr == 0x7F)
		return -EINVAL;
	*addr = ++uhci->device_addr;
	uhci_unlock(uhci);
	return 0;
}

static void
setup_port(struct uhci *uhci, uint8_t port)
{
	enum usb_speed speed;
	uint16_t status;
	int ret;

	status = uhci_r16(uhci, REG_PORTSC(port));
	if (!(status & (1 << 7)))
		return;
	if (!(status & PORTSC_CCS))
		return;
	/* XXX send port reset */
	uhci_w16(uhci, REG_PORTSC(port), PORTSC_PE);
	if (status & PORTSC_LSDA)
		speed = USB_SPEED_LOW;
	else
		speed = USB_SPEED_FULL;
	ret = usb_device_alloc(&uhci->hcd, speed, &uhci->devices[port]);
	if (ret)
	{
		TRACE("uhci: failed to create device");
		return;
	}
	usb_device_probe(uhci->devices[port]);
}

static int
setup_frame_list(struct uhci *uhci)
{
	int ret;

	ret = dma_buf_alloc(PAGE_SIZE, DMA_32BIT, &uhci->frame_list_dma);
	if (ret)
	{
		TRACE("uhci: frame list allocation failed");
		return ret;
	}
	uhci->frame_list = uhci->frame_list_dma->data;
	ret = dma_buf_alloc(PAGE_SIZE * 8, DMA_32BIT, &uhci->td_dma);
	if (ret)
	{
		TRACE("uhci: td allocation failed");
		return ret;
	}
	uhci->td = uhci->td_dma->data;
	for (size_t i = 0; i < uhci->td_dma->size / sizeof(*uhci->td); ++i)
		TAILQ_INSERT_TAIL(&uhci->free_td, &uhci->td[i], chain);
	ret = dma_buf_alloc(PAGE_SIZE, DMA_32BIT, &uhci->qh_dma);
	if (ret)
	{
		TRACE("uhci: qh allocation failed");
		return ret;
	}
	uhci->qh = uhci->qh_dma->data;
	for (size_t i = 0; i < uhci->qh_dma->size / sizeof(*uhci->qh); ++i)
		TAILQ_INSERT_TAIL(&uhci->free_qh, &uhci->qh[i], chain);
	uhci->root_qh = qh_alloc(uhci);
	if (!uhci->root_qh)
	{
		TRACE("uhci: root qh allocation failed");
		return -ENOMEM;
	}
	uhci->intr_qh = qh_alloc(uhci);
	if (!uhci->intr_qh)
	{
		TRACE("uhci: intr qh allocation failed");
		return -ENOMEM;
	}
	uhci->ctrl_qh = qh_alloc(uhci);
	if (!uhci->ctrl_qh)
	{
		TRACE("uhci: ctrl qh allocation failed");
		return -ENOMEM;
	}
	uhci->bulk_qh = qh_alloc(uhci);
	if (!uhci->bulk_qh)
	{
		TRACE("uhci: bulk qh allocation failed");
		return -ENOMEM;
	}
	uhci->bulk_qh->link = QH_T;
	uhci->bulk_qh->elem = QH_T;
	uhci->ctrl_qh->link = QH_T;
	uhci->ctrl_qh->elem = QH_Q | qh_paddr(uhci, uhci->bulk_qh);
	uhci->intr_qh->link = QH_T;
	uhci->intr_qh->elem = QH_Q | qh_paddr(uhci, uhci->ctrl_qh);
	uhci->root_qh->link = QH_T;
	uhci->root_qh->elem = QH_Q | qh_paddr(uhci, uhci->intr_qh);
	for (size_t i = 0; i < PAGE_SIZE / sizeof(*uhci->frame_list); ++i)
		uhci->frame_list[i] = (1 << 1) | qh_paddr(uhci, uhci->root_qh);
	uhci_w32(uhci, REG_FRBASEADD, pm_page_addr(uhci->frame_list_dma->pages));
	return 0;
}

static const struct usb_hcd_op
op =
{
	.get_addr = uhci_get_addr,
	.ctrl_transfer = uhci_ctrl_transfer,
	.intr_transfer = uhci_intr_transfer,
	.isoc_transfer = uhci_isoc_transfer,
};

static void
poll_intr_pipes(struct uhci *uhci)
{
	struct uhci_intr_pipe_head completed;
	struct uhci_intr_pipe *pipe;
	struct uhci_td *td;
	int ret;

	TAILQ_INIT(&completed);
	TAILQ_FOREACH(pipe, &uhci->intr_pipes, chain)
	{
		ret = qh_poll(uhci, pipe->qh, &pipe->td_list);
		if (ret == -EAGAIN)
			continue;
		if (!ret)
			ret = td_list_count_bytes(&pipe->td_list, 0);
		pipe->ret = ret;
		TAILQ_INSERT_TAIL(&completed, pipe, poll_chain);
	}
	uhci_unlock(uhci);
	while ((pipe = TAILQ_FIRST(&completed)))
	{
		TAILQ_REMOVE(&completed, pipe, poll_chain);
		pipe->pipe->fn(pipe->pipe, pipe->ret);
		TAILQ_FOREACH(td, &pipe->td_list, chain)
		{
			td->length = 0x7FF;
			td->flags = TD_FLAG_ACTIVE | TD_FLAG_CTR(1);
		}
		TAILQ_LAST(&pipe->td_list, uhci_td_head)->flags |= TD_FLAG_IOC;
		pipe->qh->elem = td_paddr(uhci, TAILQ_FIRST(&pipe->td_list));
	}
	uhci_lock(uhci);
}

static void
poll_isoc_pipes(struct uhci *uhci)
{
	struct uhci_isoc_pipe_head completed;
	struct uhci_isoc_pipe *pipe;
	struct uhci_td *td;

	TAILQ_INIT(&completed);
	TAILQ_FOREACH(pipe, &uhci->isoc_pipes, chain)
	{
		td = TAILQ_LAST(&pipe->td_lists[pipe->td_ff], uhci_td_head);
		if (td->flags & TD_FLAG_ACTIVE)
			continue;
		TAILQ_INSERT_TAIL(&completed, pipe, poll_chain);
	}
	uhci_unlock(uhci);
	while ((pipe = TAILQ_FIRST(&completed)))
	{
		/* XXX
		 * we should make sure we're doing ok for both of the lists
		 * interrupts can be received very late if kernel is under
		 * heavy load
		 */
		TAILQ_REMOVE(&completed, pipe, poll_chain);
		refill_isoc_pipe(pipe, pipe->td_ff);
		struct uhci_td_head *td_list = &pipe->td_lists[pipe->td_ff];
		size_t *td_pos = &pipe->td_pos[pipe->td_ff];
		remove_isoc_queue(uhci, td_list, *td_pos);
		TAILQ_FOREACH(td, td_list, chain)
		{
			td->length = 0x7FF;
			td->flags = TD_FLAG_IOS | TD_FLAG_ACTIVE | TD_FLAG_CTR(1);
		}
		TAILQ_LAST(td_list, uhci_td_head)->flags |= TD_FLAG_IOC;
		*td_pos = (*td_pos + pipe->td_count) % 1024;
		insert_isoc_queue(uhci, td_list, *td_pos);
		pipe->td_ff = !pipe->td_ff;
	}
	uhci_lock(uhci);
}

static void
int_handler(void *userptr)
{
	struct uhci *uhci = userptr;
	uint16_t status = uhci_r16(uhci, REG_USBSTS);

	if (!status)
		return;
	/* it's not the correct way to do this, but not asserting every
	 * interrupt make the interrupt mechanism blocked
	 */
	uhci_w16(uhci, REG_USBSTS, 0x3F);
	uhci_lock(uhci);
	poll_intr_pipes(uhci);
	poll_isoc_pipes(uhci);
	waitq_broadcast(&uhci->waitq, 0);
	uhci_unlock(uhci);
}

static void
uhci_free(struct uhci *uhci)
{
	if (!uhci)
		return;
	dma_buf_free(uhci->td_dma);
	dma_buf_free(uhci->qh_dma);
	dma_buf_free(uhci->frame_list_dma);
	mutex_destroy(&uhci->mutex);
	waitq_destroy(&uhci->waitq);
	pci_unmap(uhci->device, uhci->pci_map);
	pci_unregister_irq(uhci->device, &uhci->irq_handle);
	/* XXX intr pipes */
	/* XXX isoc pipes */
	free(uhci);
}

int
init_pci(struct pci_device *device, void *userdata)
{
	struct uhci *uhci = NULL;
	int ret;

	(void)userdata;
	uhci = malloc(sizeof(*uhci), M_ZERO);
	if (!uhci)
	{
		TRACE("uhci: uhci allocation failed");
		return -ENOMEM;
	}
	uhci->hcd.op = &op;
	uhci->hcd.userdata = uhci;
	TAILQ_INIT(&uhci->free_td);
	TAILQ_INIT(&uhci->free_qh);
	TAILQ_INIT(&uhci->intr_pipes);
	TAILQ_INIT(&uhci->isoc_pipes);
	waitq_init(&uhci->waitq);
	mutex_init(&uhci->mutex, 0);
	uhci->device = device;
	ret = pci_map_bar(device, 4, PAGE_SIZE, 0, &uhci->pci_map);
	if (ret)
	{
		TRACE("uhci: failed to init bar4");
		goto err;
	}
	uhci_w16(uhci, REG_USBCMD, CMD_HCRESET);
	while (uhci_r16(uhci, REG_USBCMD) & CMD_HCRESET) /* XXX timeout */
		;
	ret = setup_frame_list(uhci);
	if (ret)
		goto err;
	ret = pci_register_irq(device, int_handler, uhci, &uhci->irq_handle);
	if (ret)
	{
		TRACE("uhci: failed to enable irq");
		goto err;
	}
	uhci_w16(uhci, REG_USBINTR, 0x000F);
	uhci_w32(uhci, REG_USBCMD, CMD_RS);
	for (size_t i = 0; i < MAX_PORTS; ++i)
		setup_port(uhci, i);
	return 0;

err:
	uhci_free(uhci);
	return ret;
}

int
init(void)
{
	pci_probe_progif(PCI_CLASS_SERIAL_BUS,
	                 PCI_SUBCLASS_SERIAL_BUS_USB,
	                 PCI_PROGIF_SERIAL_BUS_USB_UHCI,
	                 init_pci, NULL);
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
	.name = "usb/uhci",
	.init = init,
	.fini = fini,
};
