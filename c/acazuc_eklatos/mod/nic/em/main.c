#define ENABLE_TRACE

#include "pci.h"

#include <net/if.h>

#include <errno.h>
#include <kmod.h>
#include <std.h>

/* PCI/PCI-X Family of Gigabit Ethernet Controllers Software Developer’s Manual
 * Revision 4.0
 */

/* Intel® 82574 GbE Controller Family Datasheet
 * Revision 3.3
 */

/* Intel® 82580EB/82580DB Gigabit Ethernet Controller Datasheet
 * Revision 2.50
 */

/* Intel® Ethernet Controller I210 Datasheet
 * Revision 3.7
 */

#define REG_CTRL          0x0000 /* device control */
#define REG_STATUS        0x0008 /* device status */
#define REG_EECD          0x0010 /* EEPROM/flash control & data */
#define REG_EERD          0x0014 /* EEPROM read */
#define REG_CTRL_EXT      0x0018 /* extended device control */
#define REG_FLA           0x001C /* flash access */
#define REG_MDIC          0x0020 /* MDI control */
#define REG_FCAL          0x0028 /* flow control access low */
#define REG_FCAH          0x002C /* flow control access high */
#define REG_FCT           0x0030 /* flow control type */
#define REG_VET           0x0038 /* vlan ether type */
#define REG_ICR           0x00C0 /* interrupt cause read */
#define REG_ITR           0x00C4 /* interrupt throttling */
#define REG_ICS           0x00C8 /* interrupt cause set */
#define REG_IMS           0x00D0 /* interrupt mask set */
#define REG_IMC           0x00D8 /* interrupt mask clear */
#define REG_EIAC          0x00DC /* interrupt auto clear */
#define REG_IAM           0x00E0 /* interrupt acknowledge auto make */
#define REG_IVAR          0x00E4 /* interrupt vector allocation */
#define REG_EITR(n)       (0x00E8 + (n) * 4) /* extended interrupt throttle */
#define REG_RCTL          0x0100 /* receive control */
#define REG_FCTTV         0x0170 /* flow control transmit timer value */
#define REG_TXCW          0x0178 /* transmit configuration word */
#define REG_RXCW          0x0180 /* receive configuration word */
#define REG_TCTL          0x0400 /* transmit control */
#define REG_TIPG          0x0410 /* transmit IPG */
#define REG_AIFS          0x0458 /* adaptive IFS throttle */
#define REG_LEDCTL        0x0E00 /* led control */
#define REG_EXTCNF_CTRL   0x0F00 /* extended configuration control */
#define REG_EXTCNT_SIZE   0x0F08 /* extended configuration size */
#define REG_PEOMB         0x0F10 /* PHY OEM bits */
#define REG_PBA           0x1000 /* packet buffer allocation */
#define REG_EEMNGCTL      0x1010 /* MNG EEPROM control */
#define REG_EEMNGDATA     0x1014 /* MNG EEPROM data */
#define REG_FLMNGCTL      0x1018 /* MNG flash control */
#define REG_FLMNGDATA     0x101C /* MNG flash read data */
#define REG_FLMNGCNT      0x1020 /* MNG flash read counter */
#define REG_FLASHT        0x1024 /* flash timer */
#define REG_EEWR          0x102C /* EEPROM write */
#define REG_FLSWCTL       0x1030 /* SW flash burst control */
#define REG_FLSWDATA      0x1034 /* SW flash burst data */
#define REG_FLSWCNT       0x1038 /* SW flash burst access */
#define REG_FLOP          0x103C /* flash opcode */
#define REG_FLOL          0x1040 /* fleep auto load */
#define REG_ICR0          0x1500 /* interrupt cause read */
#define REG_ICS0          0x1504 /* interrupt cause set */
#define REG_IMS0          0x1508 /* interrupt mask set */
#define REG_IMC0          0x150C /* interrupt mask clear */
#define REG_IAM0          0x1510 /* interrupt acknowledge audo mask */
#define REG_GPIE          0x1514 /* general purpose interrupt enable */
#define REG_EICS0         0x1520 /* extended interrupt cause set */
#define REG_EIMS0         0x1524 /* extended interrupt mask set */
#define REG_EIMC0         0x1528 /* extended interrupt mask clear */
#define REG_EIAC0         0x152C /* extended interrupt auto clear */
#define REG_EICR0         0x1580 /* extended interrupt cause read */
#define REG_IVAR0(n)      (0x1700 + (n) * 0x4) /* interrupt vector allocation */
#define REG_IVAR_MISC     0x1740 /* interrupt vector allocation misc */
#define REG_FCRTL         0x2160 /* flow control receive threshold low */
#define REG_FCRTH         0x2168 /* flow control receive threshold high */
#define REG_PSRCTL        0x2170 /* packet split receive control */
#define REG_RDFH          0x2410 /* receive data fifo head */
#define REG_RDFT          0x2418 /* receive data fifo tail */
#define REG_RDFHS         0x2420 /* receive data fifo head saved */
#define REG_RDFTS         0x2428 /* receive data fifo tail saved */
#define REG_RDFPC         0x2430 /* receive data fifo packet count */
#define REG_RDREG(n, off) (0x2800 + (off) + (n) * 0x100)
#define REG_RDBAL(n)      REG_RDREG(n, 0x00) /* receive descriptor base address low */
#define REG_RDBAH(n)      REG_RDREG(n, 0x04) /* receive descriptor base address high */
#define REG_RDLEN(n)      REG_RDREG(n, 0x08) /* receive descriptor length */
#define REG_RDH(n)        REG_RDREG(n, 0x10) /* receive descriptor head */
#define REG_RDT(n)        REG_RDREG(n, 0x18) /* receive descriptor tail */
#define REG_RDTR(n)       REG_RDREG(n, 0x20) /* receive delay timer */
#define REG_RXDCTL(n)     REG_RDREG(n, 0x28) /* receive descriptor control */
#define REG_RADV(n)       REG_RDREG(n, 0x2C) /* receive interrupt absolute delay value */
#define REG_RSRPD         0x2C00 /* received small packet detect interrupt */
#define REG_TXDMAC        0x3000 /* TX DMA control */
#define REG_TDFH          0x3410 /* transmit data fifo head */
#define REG_TDFT          0x3418 /* transmit data fifo tail */
#define REG_TDFHS         0x3420 /* transmit data fifo head saved */
#define REG_TDFTS         0x3428 /* transmit data fifo tail saved */
#define REG_TDFPC         0x3430 /* transmit data fifo packet count */
#define REG_TXREG(n, off) (0x3800 + (off) + (n) * 0x100)
#define REG_TDBAL(n)      REG_TXREG(n, 0x00) /* transmit descriptor base address low */
#define REG_TDBAH(n)      REG_TXREG(n, 0x04) /* transmit descriptor base address high */
#define REG_TDLEN(n)      REG_TXREG(n, 0x08) /* transmit descriptor length */
#define REG_TDH(n)        REG_TXREG(n, 0x10) /* transmit descriptor head */
#define REG_TDT(n)        REG_TXREG(n, 0x18) /* transmit descriptor tail */
#define REG_TIDV(n)       REG_TXREG(n, 0x20) /* transmit interrupt delay value */
#define REG_TXDCTL(n)     REG_TXREG(n, 0x28) /* transmit descriptor control */
#define REG_TADV(n)       REG_TXREG(n, 0x2C) /* transmit absolute interrupt delay value */
#define REG_TSPMT(n)      REG_TXREG(n, 0x30) /* TCP segmentation pad an minimum threshold */
#define REG_TARC(n)       REG_TXREG(n, 0x40) /* transmit abritration count */
#define REG_CRCERRS       0x4000 /* CRC error count */
#define REG_ALGNERRC      0x4004 /* alignement error count */
#define REG_SYMERRS       0x4008 /* symbol error count */
#define REG_RXERRC        0x400C /* RX error count */
#define REG_MPC           0x4010 /* missed packets count */
#define REG_SCC           0x4014 /* single collision count */
#define REG_ECOL          0x4018 /* excessive collisions count */
#define REG_MCC           0x401C /* multiple collision count */
#define REG_LATECOL       0x4020 /* late collisions count */
#define REG_COLC          0x4028 /* collision count */
#define REG_RERC          0x402C /* receive error count */
#define REG_DC            0x4030 /* defer count */
#define REG_TNCRS         0x4034 /* transmit with no crs */
#define REG_SEC           0x4038 /* sequence error count */
#define REG_CEXTERR       0x403C /* carrier extension error count */
#define REG_RLEC          0x4040 /* receive length error count */
#define REG_XONRXC        0x4048 /* xon received count */
#define REG_XONTXC        0x404C /* xon transmitted count */
#define REG_XOFFRXC       0x4050 /* xoff received count */
#define REG_XOFFTXC       0x4054 /* xoff transmitted count */
#define REG_FCRUC         0x4058 /* FC received unsupported count */
#define REG_PRC64         0x405C /* packets received count (64 bytes) */
#define REG_PRC127        0x4060 /* packets received count (65-127 bytes) */
#define REG_PRC255        0x4064 /* packets received count (128-255 bytes) */
#define REG_PRC511        0x4068 /* packets received count (256-511 bytes) */
#define REG_PRC1023       0x406C /* packets received count (512-1023 bytes) */
#define REG_PRC1522       0x4070 /* packets received count (1024+ bytes) */
#define REG_GPRC          0x4074 /* good packets received count */
#define REG_BPRC          0x4078 /* broadcast packets received count */
#define REG_MPRC          0x407C /* multicast packets received count */
#define REG_GPTC          0x4080 /* good packets transmitted count */
#define REG_GORCL         0x4088 /* good octets received count low */
#define REG_GORCH         0x408C /* good octets received count high */
#define REG_GOTCL         0x4090 /* good octets transmitted count low */
#define REG_GOTCH         0x4094 /* good octets transmitted count high */
#define REG_RNBC          0x40A0 /* receive no buffers count */
#define REG_RUC           0x40A4 /* receive undersize count */
#define REG_RFC           0x40A8 /* receive fragment count */
#define REG_ROC           0x40AC /* receive oversize count */
#define REG_RJC           0x40B0 /* receive jabber count */
#define REG_MGTPRC        0x40B4 /* management packets received count */
#define REG_MGTPDC        0x40B8 /* management packets dropped count */
#define REG_MGTPTC        0x40BC /* management packets transmitted count */
#define REG_TORL          0x40C0 /* total octets received low */
#define REG_TORH          0x40C4 /* total octets received high */
#define REG_TOTL          0x40C8 /* total octets transmitted low */
#define REG_TOTH          0x40CC /* total octets transmitted high */
#define REG_TPR           0x40D0 /* total packets received */
#define REG_TPT           0x40D4 /* total packets transmitted */
#define REG_PTC64         0x40D8 /* packets transmitted count (64 bytes) */
#define REG_PTC127        0x40DC /* packets transmitted count (64-127 bytes) */
#define REG_PTC255        0x40E0 /* packets transmitted count (128-255 bytes) */
#define REG_PTC511        0x40E4 /* packets transmitted count (256-511 bytes) */
#define REG_PTC1023       0x40E8 /* packets transmitted count (512-1023 bytes) */
#define REG_PTC1522       0x40EC /* packets transmitted count (1024+ bytes) */
#define REG_MPTC          0x40F0 /* multicast packets transmitted count */
#define REG_BPTC          0x40F4 /* broadcast packets transmitted count */
#define REG_TSCTC         0x40F8 /* TCP segmentation context transmitted count */
#define REG_TSCTFC        0x40FC /* TCP segmentation context transmit fail count */
#define REG_IAC           0x4100 /* interrupt assertion count */
#define REG_RXCSUM        0x5000 /* receive checksum control */
#define REG_MAVTV0        0x5010 /* management vlan tag value 0 */
#define REG_MAVTV1        0x5014 /* management vlan tag value 1 */
#define REG_MAVTV2        0x5018 /* management vlan tag value 2 */
#define REG_MAVTV3        0x501C /* management vlan tag value 3 */
#define REG_MTA           0x5200 /* multicast table array */
#define REG_RAL(n)        (0x5400 + (n) * 0x8) /* receive address low */
#define REG_RAH(n)        (0x5404 + (n) * 0x8) /* receive address high */
#define REG_VFTA          0x5600 /* vlan filter table aray */
#define REG_WUC           0x5800 /* wake up control */
#define REG_WUFC          0x5808 /* wake up filter control */
#define REG_WUS           0x5810 /* wake up status */
#define REG_MRQC          0x5818 /* multiple receive queues command */
#define REG_MANC          0x5820 /* management control */
#define REG_MFUTP01       0x5828 /* management flex UDP/TCP ports 0/1 */
#define REG_MFUTP23       0x5830 /* management flex UDP/TCP ports 2/3 */
#define REG_IPAV          0x5838 /* ip address valid */
#define REG_IP4AT         0x5840 /* IPv4 address table */
#define REG_MANC2H        0x5860 /* management control to host */
#define REG_IP6AT         0x5880 /* IPv6 address table */
#define REG_WUPL          0x5900 /* wake up packet length */
#define REG_WUPM          0x5A00 /* wake up packet memory */
#define REG_FACTPS        0x5B30 /* function active and power state to mng */
#define REG_RETA(n)       (0x5C00 + (n) * 0x4) /* redirection table */
#define REG_RSSRK(n)      (0x5C80 + (n) * 0x4) /* RSS random key */
#define REG_FFLT          0x5F00 /* flexible filter length table */
#define REG_FCRTV         0x5F40 /* flow control refresh threshold */
#define REG_FFMT          0x9000 /* flexible filter mask table */
#define REG_FTFT          0x9400 /* flexible TCO filter table */
#define REG_FFVT          0x9800 /* flexible filter filter value table */

#define CTRL_FD              (1 << 0) /* full duplex */
#define CTRL_LRST            (1 << 3) /* link reset */
#define CTRL_ASDE            (1 << 5) /* auto-speed detection enable */
#define CTRL_SLU             (1 << 6) /* set link up */
#define CTRL_ILOS            (1 << 7) /* invert loss of signal */
#define CTRL_SPEED(x)        ((x) << 8) /* speed */
#define CTRL_FRCSPD          (1 << 11) /* force speed */
#define CTRL_FRCDPLX         (1 << 12) /* force duplex */
#define CTRL_SDP0_DATA       (1 << 18) /* SDP0 data value */
#define CTRL_SDP1_DATA       (1 << 19) /* SDP1 data value */
#define CTRL_ADVD3WUC        (1 << 20) /* D3col wakeup capability advestisement enable */
#define CTRL_EN_PHY_PWR_MGMT (1 << 21) /* PHY power management enable */
#define CTRL_SDP0_IODIR      (1 << 22) /* SDP0 pin directionality */
#define CTRL_SDP1_IODIR      (1 << 23) /* SDP1 pin directionality */
#define CTRL_RST             (1 << 26) /* device reset */
#define CTRL_RFCE            (1 << 27) /* receive flow control enable */
#define CTRL_TFCE            (1 << 28) /* transmit flow control enable */
#define CTRL_DEV_RST         (1 << 29) /* device reset */
#define CTRL_VME             (1 << 30) /* vlan mode enable */
#define CTRL_PHY_RST         (1 << 31) /* phy reset */

#define ICR_TXDW     (1 << 0) /* transmit description written back */
#define ICR_TXQE     (1 << 1) /* transmit queue empty */
#define ICR_LSC      (1 << 2) /* link status change */
#define ICR_RXSEQ    (1 << 3) /* receive sequence error */
#define ICR_RXDMT0   (1 << 4) /* receive descriptor minimum threshold reached */
#define ICR_RXO      (1 << 6) /* receiver overrun */
#define ICR_RXT0     (1 << 7) /* receiver timer interrupt */
#define ICR_MDAC     (1 << 9) /* MDI/O access complete */
#define ICR_RXCFG    (1 << 10) /* receiving /C/ ordered sets */
#define ICR_PHYINT   (1 << 12) /* phy interrupt */
#define ICR_GPI_SDP6 (1 << 13) /* general purpose interrupt on SDP6 */
#define ICR_GPI_SDP7 (1 << 14) /* general purpose interrupt on SDP7 */
#define ICR_TXD_LOW  (1 << 15) /* transmit descriptor low threshold hit */
#define ICR_SRPD     (1 << 16) /* small receive packet detected */
#define ICR_ACK      (1 << 17) /* receive ACK frame detection */
#define ICR_MNG      (1 << 18) /* manageability event */
#define ICR_RXQ0     (1 << 20) /* receive queue 0 */
#define ICR_RXQ1     (1 << 21) /* receive queue 1 */
#define ICR_TXQ0     (1 << 22) /* transmit queue 0 */
#define ICR_TXQ1     (1 << 23) /* transmit queue 1 */
#define ICR_OTHER    (1 << 24) /* other interrupt */

#define ICR0_TXDW     (1 << 0) /* transmit descriptor writeback */
#define ICR0_LSC      (1 << 2) /* link status change */
#define ICR0_RXDMT0   (1 << 4) /* received descriptor minimum threshold reached */
#define ICR0_RX_MISS  (1 << 6) /* missed received */
#define ICR0_RXDW     (1 << 7) /* receive descriptor writeback */
#define ICR0_GPHY     (1 << 10) /* phy interrupt */
#define ICR0_GPI_SDP0 (1 << 11) /* GPI SDP0 */
#define ICR0_GPI_SDP1 (1 << 12) /* GPI SDP1 */
#define ICR0_GPI_SDP2 (1 << 13) /* GPI SDP2 */
#define ICR0_GPI_SDP3 (1 << 14) /* GPI SDP3 */
#define ICR0_MNG      (1 << 18) /* management event */
#define ICR0_TS       (1 << 19) /* time sync */
#define ICR0_FER      (1 << 22) /* fatal error */
#define ICR0_PCE_EX   (1 << 24) /* PCI exception */
#define ICR0_SCE      (1 << 25) /* DMA coalescing clock control event */
#define ICR0_SWD      (1 << 26) /* software watchdog */
#define ICR0_TCPT     (1 << 29) /* TCP timer */
#define ICR0_DRSTA    (1 << 30) /* device reset asserted */

#define RCTL_RST      (1 << 0) /* reset */
#define RCTL_EN       (1 << 1) /* enable */
#define RCTL_SBP      (1 << 2) /* store bad packet */
#define RCTL_UPE      (1 << 3) /* unicast promisc enable */
#define RCTL_MPE      (1 << 4) /* multicast promisc enable */
#define RCTL_LPE      (1 << 5) /* king packet enable */
#define RCTL_LBM(x)   ((x) << 6) /* loopback mode */
#define RCTL_RDMTS(x) ((x) << 8) /* receive description minimum threshold */
#define RCTL_MO(x)    ((x) << 12) /* multicast offset */
#define RCTL_BAM      (1 << 15) /* broadcast enable */
#define RCTL_BSIZE(x) ((x) << 16)
#define RCTL_VFE      (1 << 18) /* vlan filter enable */
#define RCTL_CFIEN    (1 << 19) /* canonical form indicator enable */
#define RCTL_CFI      (1 << 20) /* canonical form indicator */
#define RCTL_DPF      (1 << 22) /* discard pause frame */
#define RCTL_PMCF     (1 << 23) /* pass MAC control frames */
#define RCTL_BSEX     (1 << 25) /* buffer size extension */
#define RCTL_SECRC    (1 << 26) /* strip ethernet CRC */

#define TCTL_EN      (1 << 1) /* transmit enable */
#define TCTL_PSP     (1 << 2) /* pad short packets */
#define TCTL_CT(x)   ((x) << 3) /* collision threshold */
#define TCTL_COLD(x) (1 << 12) /* collision distance */
#define TCTL_SWXOFF  (1 << 22) /* software xoff transmission */
#define TCTL_RTLC    (1 << 24) /* re-transmit on late collision */
#define TCTL_NRTU    (1 << 25) /* no re-transmit on underrun */

#define RXD_STATUS_DD    (1 << 0) /* descriptor done */
#define RXD_STATUS_EOP   (1 << 1) /* end of packet */
#define RXD_STATUS_IXSM  (1 << 2) /* ignore checksum indication */
#define RXD_STATUS_VP    (1 << 3) /* packet is 802.1Q */
#define RXD_STATUS_TCPCS (1 << 5) /* TCP checksum calculated */
#define RXD_STATUS_IPCS  (1 << 6) /* IP checksum calculated */
#define RXD_STATUS_PIF   (1 << 7) /* passed in-exact filter */

#define RXD_ERRORS_CE   (1 << 0) /* CRC / alignement error */
#define RXD_ERRORS_SE   (1 << 1) /* symbol error */
#define RXD_ERRORS_SEQ  (1 << 2) /* sequence error */
#define RXD_ERRORS_CXE  (1 << 4) /* carrier extension error */
#define RXD_ERRORS_TCPE (1 << 5) /* TCP/UDP checksum error */
#define RXD_ERRORS_IPE  (1 << 6) /* IP checksum error */
#define RXD_ERRORS_RXE  (1 << 7) /* RX data error */

#define TXD_CMD_EOP  (1 << 0) /* end of packet */
#define TXD_CMD_IFCS (1 << 1) /* insert FCS */
#define TXD_CMD_IC   (1 << 2) /* insert checksum */
#define TXD_CMD_RS   (1 << 3) /* report status */
#define TXD_CMD_RPS  (1 << 4) /* report packet sent */
#define TXD_CMD_DEXT (1 << 5) /* extension */
#define TXD_CMD_VLE  (1 << 6) /* vlan packet enable */
#define TXD_CMD_IDE  (1 << 7) /* interrupt delay enable */

#define TXD_STA_DD (1 << 0) /* descriptor done */
#define TXD_STA_EC (1 << 1) /* excess collisions */
#define TXD_STA_LC (1 << 2) /* late collision */
#define TXD_STA_TU (1 << 3) /* transmit underrun */

#define GPIE_NSICR  (1 << 0) /* non selective interrupt clear or read */
#define GPIE_MSIX   (1 << 4) /* multiple msix */
#define GPIE_LLI(n) ((n) << 7) /* low latency credits increment rate */
#define GPIE_EIAME  (1 << 30) /* extended interrupt auto make enable */
#define GPIE_PBA    (1 << 31) /* pba support */

#define CTRL_EXT_GPI_EN(n)    ((n) << 0) /* general purpose interrupt enable */
#define CTRL_EXT_SWDPINSHI(n) ((n) << 4) /* software defined pins high nibble */
#define CTRL_EXT_SWDPIOHI(n)  ((n) << 8) /* software defined ping input or output */
#define CTRL_EXT_ASDCHK       (1 << 12) /* ASD check */
#define CTRL_EXT_EE_RST       (1 << 13) /* EEPROM reset */
#define CTRL_EXT_IPS          (1 << 14) /* invert power state */
#define CTRL_EXT_SPD_BYPS     (1 << 15) /* speed select bypass */
#define CTRL_EXT_IPS1         (1 << 16) /* invert power state bit 1 */
#define CTRL_EXT_RO_DIS       (1 << 17) /* relaxed ordering disable */
#define CTRL_EXT_DMA_DGE      (1 << 19) /* DMA dynamic gating enable */
#define CTRL_EXT_PHY_PDE      (1 << 20) /* PHY power down enable */
#define CTRL_EXT_TX_LSF       (1 << 22) /* tx LS flow */
#define CTRL_EXT_TX_LS        (1 << 23) /* tx LS */
#define CTRL_EXT_EIAME        (1 << 24) /* extended interrupt auto mask enable */
#define CTRL_EXT_IAME         (1 << 27) /* interrupt acknowledge auto make enable */
#define CTRL_EXT_DRV_LOAD     (1 << 28) /* driver loaded */
#define CTRL_EXT_ITCENA       (1 << 29) /* interrupt timers clear ena */
#define CTRL_EXT_PBAS         (1 << 31) /* PBA support */

#define STATUS_FD          (1 << 0) /* full duplex */
#define STATUS_LU          (1 << 1) /* link up */
#define STATUS_TXOFF       (1 << 4) /* transmissions paused */
#define STATUS_SPEED(n)    ((n) << 6) /* link speed setting */
#define STATUS_ASDV(n)     ((n) << 8) /* auto-speed detection value */
#define STATUS_PHYRA       (1 << 10) /* phy reset asserted */
#define STATUS_GIO_MES     (1 << 19) /* GIO master enable status */
#define STATUS_DEV_RST_SET (1 << 20) /* device reset rest */
#define STATUS_PF_RST_DONE (1 << 21) /* pf reset done */
#define STATUS_MAC_CGE     (1 << 31) /* mac clock gating enable */

#define TXDCTL_PTHRESH(n)   ((n) << 0) /* prefetch threshold */
#define TXDCTL_HTHRESH(n)   ((n) << 8) /* host threshold */
#define TXDCTL_WTHRESH(n)   ((n) << 16) /* writeback threshold */
#define TXDCTL_GRAN         (1 << 24) /* granularity */
#define TXDCTL_ENABLE       (1 << 25) /* transmit queue enable */
#define TXDCTL_SWFLSH       (1 << 26) /* transmit software flush */
#define TXDCTL_PRIORITY     (1 << 27) /* transmit queue priority */
#define TXDCTL_HWBTHRESH(n) ((n) << 28) /* transmit headwriteback threshold */

#define RXDCTL_PTHRESH(n) ((n) << 0) /* prefetch threshold */
#define RXDCTL_HTHRESH(n) ((n) << 8) /* host threshold */
#define RXDCTL_WTHRESH(n) ((n) << 16) /* writeback threshold */
#define RXDCTL_ENABLE     (1 << 25) /* receive queue enable */
#define RXDCTL_SWFLUSH    (1 << 26) /* receive software flush */

#define RXCTL_FETCH_TPH_EN (1 << 0) /* fetch TPH enable */
#define RXCTL_WB_TPH_EN    (1 << 1) /* writeback TPH enable */
#define RXCTL_HDR_TPH_EN   (1 << 2) /* header TPH enable */
#define RXCTL_P_TPH_EN     (1 << 3) /* payload TPH enable */
#define RXCTL_DCA_EN       (1 << 5) /* DCA enable */
#define RXCTL_HDR_DCA_EN   (1 << 6) /* header DCA enable */
#define RXCTL_P_DCA_EN     (1 << 7) /* payload DCA enable */
#define RXCTL_NS_EN        (1 << 8) /* no snoop enable */
#define RXCTL_RO_EN        (1 << 9) /* relaxed order enable */
#define RXCTL_WBNS_EN      (1 << 10) /* writeback no snoop enable */
#define RXCTL_WBRO_EN      (1 << 11) /* writeback relaxed order enable */
#define RXCTL_WNS_EN       (1 << 12) /* data write no snoop enable */
#define RXCTL_WRO_EN       (1 << 13) /* data relaxed order enable */
#define RXCTL_HDR_NS_EN    (1 << 14) /* receive replicated/split header no snoop enable */
#define RXCTL_HDR_RO_EN    (1 << 15) /* receive replicated/split header relaxed order enable */
#define RXCTL_CPUID(n)     (1 << 24) /* cpuid */

#define DESC_LEN 0x1000
#define DESC_COUNT 1024

enum em_hw
{
	EM_82540,
	EM_82570,
	EM_82580,
	EM_I210,
	EM_I220,
};

struct em_caps
{
	enum em_hw hw;
	uint32_t max_rxq;
	uint32_t max_txq;
};

struct em_desc
{
	union
	{
		struct
		{
			uint64_t addr;
			uint16_t length;
			uint16_t checksum;
			uint8_t status;
			uint8_t errors;
			uint16_t special;
		} rx;
		struct
		{
			uint64_t addr;
			uint16_t length;
			uint8_t cso;
			uint8_t cmd;
			uint8_t sta;
			uint8_t css;
			uint16_t special;
		} tx;
	};
};

struct em_queue
{
	struct em *em;
	struct dma_buf *descriptors_dma;
	struct em_desc *descriptors;
	struct dma_buf *buffers[DESC_COUNT];
	struct irq_handle irq_handle;
	struct waitq waitq;
	struct mutex mutex;
	size_t off;
	size_t id;
	int tx;
};

struct em
{
	struct pci_device *device;
	struct pci_map *pci_map;
	struct netif *netif;
	struct em_queue **rx_queues;
	struct em_queue **tx_queues;
	size_t rx_queues_count;
	size_t tx_queues_count;
	struct irq_handle irq_handle;
	const struct em_caps *caps;
};

static inline uint32_t
em_read(struct em *em, uint32_t reg)
{
	return pci_r32(em->pci_map, reg);
}

static inline void
em_write(struct em *em, uint32_t reg, uint32_t val)
{
	pci_w32(em->pci_map, reg, val);
}

static void
queue_lock(struct em_queue *queue)
{
	mutex_lock(&queue->mutex);
}

static void
queue_unlock(struct em_queue *queue)
{
	mutex_unlock(&queue->mutex);
}

int
emit_pkt(struct netif *netif, struct netpkt *pkt)
{
	struct em *em = netif->userdata;
	struct em_queue *queue;
	struct em_desc *txd;

	if (pkt->len >= DESC_LEN)
		return -ENOBUFS;
	if (!(em_read(em, REG_STATUS) & STATUS_LU))
		return -ENOBUFS;
	queue = em->tx_queues[curcpu()->id % em->tx_queues_count];
	queue_lock(queue);
	while (queue->descriptors[queue->off].tx.sta & TXD_STA_DD)
	{
		int ret = waitq_wait_tail_mutex(&queue->waitq, &queue->mutex,
		                                NULL);
		if (ret)
		{
			queue_unlock(queue);
			return ret;
		}
	}
	memcpy(queue->buffers[queue->off]->data, pkt->data, pkt->len);
	txd = &queue->descriptors[queue->off];
	txd->tx.addr = pm_page_addr(queue->buffers[queue->off]->pages);
	txd->tx.length = pkt->len;
	txd->tx.cso = 0;
	txd->tx.cmd = TXD_CMD_EOP | TXD_CMD_IFCS;
	txd->tx.sta = 0;
	txd->tx.css = 0;
	txd->tx.special = 0;
	queue->off = (queue->off + 1) % DESC_COUNT;
	em->netif->stats.tx_packets++;
	em->netif->stats.tx_bytes += pkt->len;
	em_write(em, REG_TDT(queue->id), queue->off);
	queue_unlock(queue);
	return 0;
}

void
update_rx_filter(struct netif *netif)
{
	struct em *em = netif->userdata;
	uint32_t rctl;

	rctl = RCTL_BSIZE(0);
	if (netif->flags & IFF_UP)
	{
		rctl |= RCTL_EN;
		if (netif->flags & IFF_BROADCAST)
			rctl |= RCTL_BAM;
		/* XXX multicast */
		if (netif->flags & IFF_PROMISC)
			rctl |= RCTL_UPE;
		if (netif->flags & IFF_ALLMULTI)
			rctl |= RCTL_MPE;
	}
	em_write(em, REG_RCTL, rctl);
}

const struct netif_op
netif_op =
{
	.emit = emit_pkt,
	.update_rx_filter = update_rx_filter,
};

void
rx_desc(struct em_queue *queue, struct em_desc *desc, size_t i)
{
	struct netpkt *netpkt;
	struct em *em = queue->em;
	size_t len;

	if (desc->rx.errors)
	{
		em->netif->stats.rx_errors++;
		return;
	}
	len = desc->rx.length;
	em->netif->stats.rx_packets++;
	em->netif->stats.rx_bytes += len;
	netpkt = netpkt_alloc(len);
	if (!netpkt)
		panic("em: failed to allocate packet\n");
	memcpy(netpkt->data, queue->buffers[i]->data, len);
	ether_input(em->netif, netpkt);
}

void
rx_pkt(struct em_queue *queue)
{
	while (1)
	{
		struct em_desc *desc = &queue->descriptors[queue->off];
		if (!(desc->rx.status & RXD_STATUS_DD))
			return;
		rx_desc(queue, desc, queue->off);
		desc->rx.addr = pm_page_addr(queue->buffers[queue->off]->pages);
		desc->rx.length = DESC_LEN;
		desc->rx.checksum = 0;
		desc->rx.status = 0;
		desc->rx.errors = 0;
		desc->rx.special = 0;
		queue->off = (queue->off + 1) % DESC_COUNT;
	}
}

void
int_handler(void *userdata)
{
	struct em *em = userdata;
	uint32_t icr;

	icr = em_read(em, REG_ICR);
	if (!icr)
		return;
#if 0
	TRACE("em int %08" PRIx32, icr);
#endif
	if (icr & ICR_LSC)
	{
		if (em_read(em, REG_STATUS) & STATUS_LU)
			em->netif->flags |= IFF_RUNNING;
		else
			em->netif->flags &=  ~IFF_RUNNING;
	}
	if (icr & ICR_RXT0)
		rx_pkt(em->rx_queues[0]);
	if (icr & ICR_TXDW)
		waitq_broadcast(&em->tx_queues[0]->waitq, 0);
}

void
admin_int_handler(void *userdata)
{
	struct em *em = userdata;
	uint32_t icr;

	if (em->caps->hw >= EM_82580)
		icr = em_read(em, REG_ICR0);
	else
		icr = em_read(em, REG_ICR);
	if (!icr)
		return;
#if 0
	TRACE("em admin int %08" PRIx32, icr);
#endif
	if (icr & ICR0_LSC)
	{
		if (em_read(em, REG_STATUS) & STATUS_LU)
			em->netif->flags |= IFF_RUNNING;
		else
			em->netif->flags &=  ~IFF_RUNNING;
	}
}

void
queue_int_handler(void *userdata)
{
	struct em_queue *queue = userdata;
	struct em *em = queue->em;
	uint32_t icr;

	if (em->caps->hw >= EM_82580)
	{
		icr = em_read(em, REG_ICR0);
		if (!icr)
			return;
#if 0
		TRACE("em queue int 0x%08" PRIx32, icr);
#endif
		if (icr & ICR0_TXDW)
			waitq_broadcast(&em->tx_queues[queue->id]->waitq, 0);
		if (icr & ICR0_RXDW)
			rx_pkt(queue);
	}
	else
	{
		icr = em_read(em, REG_ICR);
		if (!icr)
			return;
#if 0
		TRACE("em queue int 0x%08" PRIx32, icr);
#endif
		if (queue->tx)
			waitq_broadcast(&em->tx_queues[queue->id]->waitq, 0);
		else
			rx_pkt(queue);
	}
}

void
clear_interrupts(struct em *em)
{
	if (em->caps->hw >= EM_82580)
		em_write(em, REG_EIMC0, 0xFFFFFFFF);
	else
		em_write(em, REG_IMC, 0xFFFFFFFF);
}

int
reset_nic(struct em *em)
{
	static const struct timespec delay = {0, 50000};
	uint32_t rst;

	if (em->caps->hw >= EM_82580)
		rst = CTRL_DEV_RST;
	else
		rst = CTRL_RST;
	em_write(em, REG_CTRL, em_read(em, REG_CTRL) | rst);
	for (size_t i = 0; i < 1000; ++i)
	{
		if (!(em_read(em, REG_CTRL) & rst))
			return 0;
		spinsleep(&delay);
	}
	return -EINVAL;
}

int
queue_alloc_descriptors(struct em_queue *queue)
{
	int ret;

	ret = dma_buf_alloc(sizeof(struct em_desc) * DESC_COUNT,
	                    0,
	                    &queue->descriptors_dma);
	if (ret)
	{
		TRACE("em: descriptors allocation failed");
		return ret;
	}
	queue->descriptors = queue->descriptors_dma->data;
	for (size_t i = 0; i < DESC_COUNT; ++i)
	{
		ret = dma_buf_alloc(PAGE_SIZE, 0, &queue->buffers[i]);
		if (ret)
		{
			TRACE("em: buffers allocation failed");
			return ret;
		}
		struct em_desc *desc = &queue->descriptors[i];
		if (queue->tx)
		{
			desc->tx.addr = pm_page_addr(queue->buffers[i]->pages);
			desc->tx.length = 0;
			desc->tx.cso = 0;
			desc->tx.cmd = 0;
			desc->tx.sta = 0;
			desc->tx.css = 0;
			desc->tx.special = 0;
		}
		else
		{
			desc->rx.addr = pm_page_addr(queue->buffers[i]->pages);
			desc->rx.length = DESC_LEN;
			desc->rx.checksum = 0;
			desc->rx.status = 0;
			desc->rx.errors = 0;
			desc->rx.special = 0;
		}
	}
	return 0;
}

void
queue_free(struct em_queue *queue)
{
	if (!queue)
		return;
	for (size_t i = 0; i < DESC_COUNT; ++i)
		dma_buf_free(queue->buffers[i]);
	dma_buf_free(queue->descriptors_dma);
	waitq_destroy(&queue->waitq);
	mutex_destroy(&queue->mutex);
	free(queue);
}

int
queue_alloc(struct em *em, size_t id, int tx, struct em_queue **queuep)
{
	struct em_queue *queue = NULL;
	int ret;

	queue = malloc(sizeof(*queue), M_ZERO);
	if (!queue)
		return -ENOMEM;
	queue->em = em;
	queue->id = id;
	queue->tx = tx;
	waitq_init(&queue->waitq);
	mutex_init(&queue->mutex, 0);
	ret = queue_alloc_descriptors(queue);
	if (ret)
		goto err;
	if (tx)
	{
		em_write(em, REG_TDBAL(queue->id), pm_page_addr(queue->descriptors_dma->pages));
#if __SIZE_WIDTH__ == 64
		em_write(em, REG_TDBAH(queue->id), pm_page_addr(queue->descriptors_dma->pages) >> 32);
#else
		em_write(em, REG_TDBAH(queue->id), 0);
#endif
		em_write(em, REG_TDLEN(queue->id), DESC_COUNT * sizeof(struct em_desc));
		em_write(em, REG_TDH(queue->id), 0);
		em_write(em, REG_TDT(queue->id), 0);
		if (em->caps->hw >= EM_82580)
		{
			em_write(em, REG_TXDCTL(queue->id), TXDCTL_ENABLE);
			while (!(em_read(em, REG_TXDCTL(queue->id)) & TXDCTL_ENABLE)) /* XXX timeout */
				;
		}
	}
	else
	{
		em_write(em, REG_RDBAL(queue->id), pm_page_addr(queue->descriptors_dma->pages));
#if __SIZE_WIDTH__ == 64
		em_write(em, REG_RDBAH(queue->id), pm_page_addr(queue->descriptors_dma->pages) >> 32);
#else
		em_write(em, REG_RDBAH(queue->id), 0);
#endif
		em_write(em, REG_RDH(queue->id), 0);
		em_write(em, REG_RDLEN(queue->id), DESC_COUNT * sizeof(struct em_desc));
		if (em->caps->hw >= EM_82580)
		{
			em_write(em, REG_RXDCTL(queue->id), RXDCTL_ENABLE);
			while (!(em_read(em, REG_RXDCTL(queue->id)) & RXDCTL_ENABLE)) /* XXX timeout */
				;
		}
		em_write(em, REG_RDT(queue->id), DESC_COUNT - 1);
	}
	if (em->caps->hw >= EM_82580)
	{
		if (!tx)
		{
			ret = pci_register_irq(em->device, queue_int_handler,
			                       queue, &queue->irq_handle);
			if (ret)
			{
				TRACE("em: failed to register queue IRQ");
				goto err;
			}
		}
		uint32_t reg = REG_IVAR0(id / 2);
		uint32_t shift = 0;
		if (tx)
			shift += 1;
		shift += (id & 1) * 2;
		shift *= 8;
		uint8_t vector = tx ? em->rx_queues[id]->irq_handle.msix.vector : queue->irq_handle.msix.vector;
		em_write(em, reg, em_read(em, reg) | ((vector | 0x80) << shift));
	}
	else if (em->caps->hw >= EM_82570)
	{
		ret = pci_register_irq(em->device, queue_int_handler,
		                       queue, &queue->irq_handle);
		if (ret)
		{
			TRACE("em: failed to register queue IRQ");
			goto err;
		}
		uint32_t shift = 0;
		if (tx)
			shift += 2;
		shift += id;
		shift *= 4;
		em_write(em, REG_IVAR, em_read(em, REG_IVAR)
		                   | (queue->irq_handle.msix.vector << shift)
		                   | (1 << (shift + 3)));
	}
	*queuep = queue;
	return 0;

err:
	queue_free(queue);
	return ret;
}

void
em_free(struct em *em)
{
	if (!em)
		return;
	netif_free(em->netif);
	if (em->rx_queues)
	{
		for (size_t i = 0; i < em->rx_queues_count; ++i)
			queue_free(em->rx_queues[i]);
		free(em->rx_queues);
	}
	if (em->tx_queues)
	{
		for (size_t i = 0; i < em->tx_queues_count; ++i)
			queue_free(em->tx_queues[i]);
		free(em->tx_queues);
	}
	pci_unmap(em->device, em->pci_map);
	free(em);
}

int
init_pci(struct pci_device *device, const struct em_caps *caps)
{
	struct em *em = NULL;
	int ret;

	if (caps->hw >= EM_82570 && !device->msix)
	{
		TRACE("em: no MSIX");
		return -EINVAL;
	}
	em = malloc(sizeof(*em), M_ZERO);
	if (!em)
	{
		TRACE("em: em allocation failed");
		return -ENOMEM;
	}
	em->caps = caps;
	ret = netif_alloc("eth", &netif_op, &em->netif);
	if (ret)
	{
		TRACE("em: netif creation failed");
		goto err;
	}
	em->netif->flags = IFF_UP | IFF_BROADCAST;
	em->netif->userdata = em;
	em->device = device;
	ret = pci_map_bar(device, 0, PAGE_SIZE * 0xA, 0, &em->pci_map);
	if (ret)
	{
		TRACE("em: failed to init bar0");
		goto err;
	}
	clear_interrupts(em);
	ret = reset_nic(em);
	if (ret)
	{
		TRACE("em: failed to reset device");
		goto err;
	}
	clear_interrupts(em);
	if (em->caps->hw >= EM_82580)
		em_write(em, REG_GPIE, GPIE_NSICR | GPIE_MSIX | GPIE_EIAME | GPIE_PBA);
	else if (em->caps->hw >= EM_82570)
		em_write(em, REG_CTRL_EXT, em_read(em, REG_CTRL_EXT) | CTRL_EXT_PBAS);
	uint32_t ral = em_read(em, REG_RAL(0));
	uint32_t rah = em_read(em, REG_RAH(0));
	em->netif->ether.addr[0] = ral >> 0;
	em->netif->ether.addr[1] = ral >> 8;
	em->netif->ether.addr[2] = ral >> 16;
	em->netif->ether.addr[3] = ral >> 24;
	em->netif->ether.addr[4] = rah >> 0;
	em->netif->ether.addr[5] = rah >> 8;
	if (em->caps->hw >= EM_82570)
	{
		ret = pci_register_irq(em->device, admin_int_handler, em,
		                       &em->irq_handle);
		if (ret)
		{
			TRACE("em: failed to register admin IRQ");
			goto err;
		}
		if (em->caps->hw >= EM_82580)
			em_write(em, REG_IVAR_MISC, (0x80 | em->irq_handle.msix.vector) << 8);
	}
	else
	{
		ret = pci_register_irq(em->device, int_handler, em,
		                       &em->irq_handle);
		if (ret)
		{
			TRACE("em: failed to register IRQ");
			goto err;
		}
	}
	em->rx_queues_count = g_ncpus;
	if (em->rx_queues_count > em->caps->max_rxq)
		em->rx_queues_count = em->caps->max_rxq;
	em->rx_queues = malloc(sizeof(*em->rx_queues) * em->rx_queues_count, M_ZERO);
	if (!em->rx_queues)
	{
		TRACE("em: rx queues allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	for (size_t i = 0; i < em->rx_queues_count; ++i)
	{
		ret = queue_alloc(em, i, 0, &em->rx_queues[i]);
		if (ret)
			goto err;
	}
	em->tx_queues_count = g_ncpus;
	if (em->tx_queues_count > em->caps->max_txq)
		em->tx_queues_count = em->caps->max_txq;
	em->tx_queues = malloc(sizeof(*em->tx_queues) * em->tx_queues_count, M_ZERO);
	if (!em->tx_queues)
	{
		TRACE("em: tx queues allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	for (size_t i = 0; i < em->tx_queues_count; ++i)
	{
		ret = queue_alloc(em, i, 1, &em->tx_queues[i]);
		if (ret)
			goto err;
	}
	em_write(em, REG_ITR, 0);
	em_write(em, REG_TXDMAC, 0);
	update_rx_filter(em->netif);
	em_write(em, REG_TCTL, TCTL_EN);
	em_write(em, REG_CTRL_EXT, em_read(em, REG_CTRL_EXT) | CTRL_EXT_IAME);
	if (em->caps->hw >= EM_82580)
	{
		em_write(em, REG_EIAC0, 0xFFFFFFFF);
		em_write(em, REG_EIMS0, 0xFFFFFFFF);
		em_write(em, REG_IMS0, 0xFFFFFFFF);
	}
	else if (em->caps->hw >= EM_82570)
	{
		em_write(em, REG_EIAC, ICR_RXQ0 | ICR_RXQ1 | ICR_TXQ0 | ICR_TXQ1);
		em_write(em, REG_IMS, ICR_RXQ0 | ICR_RXQ1 | ICR_TXQ0 | ICR_TXQ1);
	}
	else
	{
		em_write(em, REG_IMS, ICR_TXDW | ICR_RXT0);
	}
	if (em_read(em, REG_STATUS) & STATUS_LU)
		em->netif->flags |= IFF_RUNNING;
	return 0;

err:
	em_free(em);
	return ret;
}

int
init_82540(struct pci_device *device, void *userdata)
{
	static const struct em_caps caps =
	{
		.hw = EM_82540,
		.max_rxq = 1,
		.max_txq = 1,
	};
	(void)userdata;
	return init_pci(device, &caps);
}

int
init_82574(struct pci_device *device, void *userdata)
{
	static const struct em_caps caps =
	{
		.hw = EM_82570,
		.max_rxq = 2,
		.max_txq = 2,
	};
	(void)userdata;
	return init_pci(device, &caps);
}

int
init_i225(struct pci_device *device, void *userdata)
{
	static const struct em_caps caps =
	{
		.hw = EM_I220,
		.max_rxq = 4,
		.max_txq = 4,
	};
	(void)userdata;
	return init_pci(device, &caps);
}

int
init(void)
{
	pci_probe(0x8086, 0x100E, init_82540, NULL);
	pci_probe(0x8086, 0x10D3, init_82574, NULL);
	pci_probe(0x8086, 0x15F2, init_i225, NULL);
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
	.name = "em",
	.init = init,
	.fini = fini,
};
