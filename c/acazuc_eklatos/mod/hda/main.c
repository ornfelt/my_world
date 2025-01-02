#define ENABLE_TRACE

#include "snd.h"
#include "pci.h"

#include <random.h>
#include <time.h>
#include <file.h>
#include <kmod.h>
#include <uio.h>
#include <vfs.h>
#include <std.h>

/* Intel ® I/O Controller Hub 7 (ICH7)/
 * Intel ® High Definition Audio/AC’97
 *
 * High Definition Audio Specification
 * Revision 1.0a

 * ALC269 HIGH DEFINITION AUDIO CODEC WITH
 * EMBEDDED CLASS D SPEAKER AMPLIFIER
 * DATASHEET
 * Rev. 1.5
 * 20 July 2009
 */

#define REG_GCAP         0x00
#define REG_VMIN         0x02
#define REG_VMAJ         0x03
#define REG_OUTPAY       0x04
#define REG_INPAY        0x06
#define REG_GCTL         0x08
#define REG_WAKEEN       0x0C
#define REG_STATESTS     0x0E
#define REG_GSTS         0x10
#define REG_OUTSTRMPAY   0x18
#define REG_INSTRMPAY    0x1A
#define REG_INTCTL       0x20
#define REG_INTSTS       0x24
#define REG_WALCLK       0x30
#define REG_SSYNC        0x34
#define REG_CORBLBASE    0x40
#define REG_CORBUBASE    0x44
#define REG_CORBWP       0x48
#define REG_CORBRP       0x4A
#define REG_CORBCTL      0x4C
#define REG_CORBRST      0x4D
#define REG_CORBSIZE     0x4E
#define REG_RIRBLBASE    0x50
#define REG_RIRBUBASE    0x54
#define REG_RIRBWP       0x58
#define REG_RINTCNT      0x5A
#define REG_RIRBCTL      0x5C
#define REG_RIRBSTS      0x5D
#define REG_RIRBSIZE     0x5E
#define REG_IC           0x60
#define REG_IR           0x64
#define REG_IRS          0x68
#define REG_DPLBASE      0x70
#define REG_DPUBASE      0x74
#define REG_SDCTL(n)    (0x80 + (n) * 0x20)
#define REG_SDID(n)     (0x82 + (n) * 0x20)
#define REG_SDSTS(n)    (0x83 + (n) * 0x20)
#define REG_SDLPIB(n)   (0x84 + (n) * 0x20)
#define REG_SDCBL(n)    (0x88 + (n) * 0x20)
#define REG_SDLVI(n)    (0x8C + (n) * 0x20)
#define REG_SDFIFOW(n)  (0x8E + (n) * 0x20)
#define REG_SDFIFOS(n)  (0x90 + (n) * 0x20)
#define REG_SDFMT(n)    (0x92 + (n) * 0x20)
#define REG_SDBDPL(n)   (0x98 + (n) * 0x20)
#define REG_SDBDPU(n)   (0x9C + (n) * 0x20)

#define GCTL_CRST (1 << 0) /* reset */
#define GCTL_FCLT (1 << 1) /* flush control */
#define GCTL_AURE (1 << 8) /* accept unsolicited response enable */

#define INTCTL_SIE(n) ((1 << (n)) & 0xFF) /* stream interrupt enable */
#define INTCTL_CIE    (1 << 30)           /* controller interrupt enable */
#define INTCTL_GIE    (1 << 31)           /* global interrupt enable */

#define SDFMT_CHAN(n) ((uint16_t)((n) & 0xF) <<  0) /* number of channels */
#define SDFMT_BITS(n) ((uint16_t)((n) & 0x7) <<  4) /* bits per sample */
#define SDFMT_DIV(n)  ((uint16_t)((n) & 0x7) <<  8) /* base rate divisor */
#define SDFMT_MUL(n)  ((uint16_t)((n) & 0x7) << 11) /* base rate multiplier */
#define SDFMT_BASE(n) ((uint16_t)((n) & 0x1) << 14) /* 0: 48kHz, 1: 44.1kHz */
#define SDFMT_TYPE(n) ((uint16_t)((n) & 0x1) << 15) /* 0: PCM, 1: non-PCM */

#define SDFMT_BITS_8  SDFMT_BITS(0)
#define SDFMT_BITS_16 SDFMT_BITS(1)
#define SDFMT_BITS_32 SDFMT_BITS(4)

#define SDFMT_DIV_1 SDFMT_DIV(0)
#define SDFMT_DIV_2 SDFMT_DIV(1)
#define SDFMT_DIV_3 SDFMT_DIV(2)
#define SDFMT_DIV_4 SDFMT_DIV(3)
#define SDFMT_DIV_5 SDFMT_DIV(4)
#define SDFMT_DIV_6 SDFMT_DIV(5)
#define SDFMT_DIV_7 SDFMT_DIV(6)
#define SDFMT_DIV_8 SDFMT_DIV(7)

#define SDFMT_MUL_1 SDFMT_MUL(0)
#define SDFMT_MUL_2 SDFMT_MUL(1)
#define SDFMT_MUL_3 SDFMT_MUL(2)
#define SDFMT_MUL_4 SDFMT_MUL(3)

#define SDFMT_BASE_48 SDFMT_BASE(0)
#define SDFMT_BASE_41 SDFMT_BASE(1)

#define SDFMT_TYPE_PCM     SDFMT_TYPE(0)
#define SDFMT_TYPE_NON_PCM SDFMT_TYPE(1)

#define SDFMT_STEREO_16_48_PCM (SDFMT_CHAN(1) \
                              | SDFMT_BITS_16 \
                              | SDFMT_DIV_1 \
                              | SDFMT_MUL_1 \
                              | SDFMT_BASE_48 \
                              | SDFMT_TYPE(0))

#define SDCTL_SRST (1 << 0) /* reset */
#define SDCTL_RUN  (1 << 1) /* run */
#define SDCTL_IOCE (1 << 2) /* interrupt on completion enable */
#define SDCTL_FEIE (1 << 3) /* fifo error interrupt enable */
#define SDCTL_DEIE (1 << 4) /* descriptor error interrupt enable */

#define SDID_STRM(n) ((n) << 4) /* stream number */

#define SDSTS_BCI     (1 << 2) /* buffer completion interrupt status */
#define SDSTS_FIFOE   (1 << 3) /* fifo error */
#define SDSTS_DESCE   (1 << 4) /* desc error */
#define SDSTS_FIFORDY (1 << 5) /* fifo ready */

#define VERB_CODEC(v) ((uint32_t)((v) & 0x000F) << 28)
#define VERB_NODE(v)  ((uint32_t)((v) & 0x00FF) << 20)
#define VERB_CMD(v)   ((uint32_t)((v) & 0x0FFF) <<  8)
#define VERB_DATA(v)  ((uint32_t)((v) & 0xFFFF) <<  0)

#define VERB_CMD_GET_PARAM     VERB_CMD(0xF00) /* get parameter */
#define VERB_CMD_GET_CSC       VERB_CMD(0xF01) /* get connection select control */
#define VERB_CMD_SET_CSC       VERB_CMD(0x701) /* set connection select control */
#define VERB_CMD_GET_CLEC      VERB_CMD(0xF02) /* get connection list entry control */
#define VERB_CMD_GET_PR_ST     VERB_CMD(0xF03) /* get processing state */
#define VERB_CMD_SET_PR_ST     VERB_CMD(0x703) /* set processing state */
#define VERB_CMD_GET_COE_ID    VERB_CMD(0xD00) /* get coefficient index */
#define VERB_CMD_SET_COE_ID    VERB_CMD(0x500) /* set coefficient index */
#define VERB_CMD_SET_PR_COE    VERB_CMD(0xC00) /* get processing coefficient */
#define VERB_CMD_GET_PR_COE    VERB_CMD(0x400) /* set processing coefficient */
#define VERB_CMD_GET_AMP_GAIN  VERB_CMD(0xB00) /* get amplifier gain / mute */
#define VERB_CMD_SET_AMP_GAIN  VERB_CMD(0x300) /* set amplifier gain / mute */
#define VERB_CMD_GET_CONV_FMT  VERB_CMD(0xA00) /* get converter format */
#define VERB_CMD_SET_CONV_FMT  VERB_CMD(0x200) /* set converter format */
#define VERB_CMD_GET_SPDIF_CC  VERB_CMD(0xF0D) /* get S/PDIF converter control */
#define VERB_CMD_SET_SPDIF_CC1 VERB_CMD(0x70D) /* set S/PDIF converter control 1 */
#define VERB_CMD_SET_SPDIF_CC2 VERB_CMD(0x70E) /* set S/PDIF converter control 2 */
#define VERB_CMD_SET_SPDIF_CC3 VERB_CMD(0x73E) /* set S/PDIF converter control 3 */
#define VERB_CMD_SET_SPDIF_CC4 VERB_CMD(0x73F) /* set S/PDIF converter control 4 */
#define VERB_CMD_GET_PWR_ST    VERB_CMD(0xF05) /* get power state */
#define VERB_CMD_SET_PWR_ST    VERB_CMD(0x705) /* set power state */
#define VERB_CMD_GET_CONV_CTL  VERB_CMD(0xF06) /* get converter control */
#define VERB_CMD_SET_CONV_CTL  VERB_CMD(0x706) /* set converter control */
#define VERB_CMD_GET_SDI_SEL   VERB_CMD(0xF04) /* get SDI select */
#define VERB_CMD_SET_SDI_SEL   VERB_CMD(0x704) /* set SDI select */
#define VERB_CMD_GET_PIN_WC    VERB_CMD(0xF07) /* get pin widget control */
#define VERB_CMD_SET_PIN_WC    VERB_CMD(0x707) /* set pin widget control */
#define VERB_CMD_GET_CONN_SC   VERB_CMD(0xF08) /* get connection select control */
#define VERB_CMD_SET_CONN_SC   VERB_CMD(0x708) /* set connection select control */
#define VERB_CMD_GET_PIN_SENSE VERB_CMD(0xF09) /* get pin sense */
#define VERB_CMD_SET_PIN_SENSE VERB_CMD(0x709) /* set pin sense */
#define VERB_CMD_GET_EAPD_EN   VERB_CMD(0xF0C) /* get EAPD/BTL enable */
#define VERB_CMD_SET_EAPD_EN   VERB_CMD(0x70C) /* set EAPD/BTL enable */
#define VERB_CMD_GET_GPI_DATA  VERB_CMD(0xF10) /* get GPI data */
#define VERB_CMD_SET_GPI_DATA  VERB_CMD(0x710) /* set GPI data */
#define VERB_CMD_GET_GPI_WEM   VERB_CMD(0xF11) /* get GPI wake enable mask */
#define VERB_CMD_SET_GPI_WEM   VERB_CMD(0x711) /* set GPI wake enable mask */
#define VERB_CMD_GET_GPI_UEM   VERB_CMD(0xF12) /* get GPI unsolicited enable mask */
#define VERB_CMD_SET_GPI_UEM   VERB_CMD(0x712) /* set GPI unsolicited enable mask */
#define VERB_CMD_GET_GPI_SM    VERB_CMD(0xF13) /* get GPI sticky mask */
#define VERB_CMD_SET_GPI_SM    VERB_CMD(0x713) /* set GPI sticky mask */
#define VERB_CMD_GET_GPO_DATA  VERB_CMD(0xF14) /* get GPO data */
#define VERB_CMD_SET_GPU_DATA  VERB_CMD(0x714) /* set GPO data */
#define VERB_CMD_GET_GPIO_DATA VERB_CMD(0xF15) /* get GPIO data */
#define VERB_CMD_SET_GPIO_DATA VERB_CMD(0x715) /* set GPIO data */
#define VERB_CMD_GET_GPIO_EM   VERB_CMD(0xF16) /* get GPIO enable mask */
#define VERB_CMD_SET_GPIO_EM   VERB_CMD(0x716) /* set GPIO enable mask */
#define VERB_CMD_GET_GPIO_DIR  VERB_CMD(0xF17) /* get GPIO direction */
#define VERB_CMD_SET_GPIO_DIR  VERB_CMD(0x717) /* set GPIO direction */
#define VERB_CMD_GET_GPIO_WEM  VERB_CMD(0xF18) /* get GPIO wake enable mask */
#define VERB_CMD_SET_GPIO_WEM  VERB_CMD(0x718) /* set GPIO wake enable mask */
#define VERB_CMD_GET_GPIO_UEM  VERB_CMD(0xF19) /* get GPIO unsolicited enable mask */
#define VERB_CMD_SET_GPIO_UEM  VERB_CMD(0x719) /* set GPIO unsolicited enable mask */
#define VERB_CMD_GET_GPIO_SM   VERB_CMD(0xF1A) /* get GPIO sticky mask */
#define VERB_CMD_SET_GPIO_SM   VERB_CMD(0x71A) /* set GPIO sticky mask */
#define VERB_CMD_GET_BEEP_GEN  VERB_CMD(0xF0A) /* get beep generation */
#define VERB_CMD_SET_BEEP_GEN  VERB_CMD(0x70A) /* set beep generation */
#define VERB_CMD_GET_VOL_KNOB  VERB_CMD(0xF0F) /* get volume knob */
#define VERB_CMD_SET_VOL_KNOB  VERB_CMD(0x70F) /* set volume knob */
#define VERB_CMD_GET_IMP_ID    VERB_CMD(0xF20) /* get implementation identification */
#define VERB_CMD_SET_IMP_ID1   VERB_CMD(0x720) /* set implementation identification 1 */
#define VERB_CMD_SET_IMP_ID2   VERB_CMD(0x721) /* set implementation identification 2 */
#define VERB_CMD_SET_IMP_ID3   VERB_CMD(0x722) /* set implementation identification 3 */
#define VERB_CMD_SET_IMP_ID4   VERB_CMD(0x723) /* set implementation identification 4 */
#define VERB_CMD_GET_CONF_DFL  VERB_CMD(0xF1C) /* get configuration default */
#define VERB_CMD_SET_CONF_DFL1 VERB_CMD(0x71C) /* set configuration default 1 */
#define VERB_CMD_SET_CONF_DFL2 VERB_CMD(0x71D) /* set configuration default 2 */
#define VERB_CMD_SET_CONF_DFL3 VERB_CMD(0x71E) /* set configuration default 3 */
#define VERB_CMD_SET_CONF_DFL4 VERB_CMD(0x71F) /* set configuration default 4 */
#define VERB_CMD_GET_STR_CTL   VERB_CMD(0xF24) /* get stripe control */
#define VERB_CMD_SET_STR_CTL   VERB_CMD(0x724) /* set stripe control */
#define VERB_CMD_FN_RESET      VERB_CMD(0x7FF) /* function reset */
#define VERB_CMD_GET_ELD_DATA  VERB_CMD(0xF2F) /* get ELF data */
#define VERB_CMD_GET_CONV_NCH  VERB_CMD(0xF2D) /* get converter channel count */
#define VERB_CMD_SET_CONV_NCH  VERB_CMD(0x72D) /* set converter channel count */
#define VERB_CMD_GET_DIP       VERB_CMD(0xF2E) /* get data island packet */
#define VERB_CMD_GET_DIP_INDEX VERB_CMD(0xF30) /* get data island packet index */
#define VERB_CMD_SET_DIP_INDEX VERB_CMD(0x739) /* set data island packet index */
#define VERB_CMD_GET_DIP_DATA  VERB_CMD(0xF31) /* get data island packet data */
#define VERB_CMD_SET_DIP_DATA  VERB_CMD(0x731) /* set data island packet data */
#define VERB_CMD_GET_DIP_XCTL  VERB_CMD(0xF32) /* get data island packet transmit control */
#define VERB_CMD_SET_DIP_XCTL  VERB_CMD(0x732) /* set data island packet transmit control */
#define VERB_CMD_GET_CP_CTL    VERB_CMD(0xF33) /* get content protection control */
#define VERB_CMD_SET_CP_CTL    VERB_CMD(0x733) /* set content protection control */
#define VERB_CMD_GET_ASP_CH_M  VERB_CMD(0xF34) /* get ASP channel mapping */
#define VERB_CMD_SET_ASP_CH_M  VERB_CMD(0x734) /* set ASP channel mapping */

#define PARAM_VENDOR_ID    VERB_DATA(0x00) /* vendor ID */
#define PARAM_REVISION_ID  VERB_DATA(0x02) /* revision ID */
#define PARAM_SUB_NODE_NB  VERB_DATA(0x04) /* subordinate node count */
#define PARAM_FN_GRP_TYPE  VERB_DATA(0x05) /* function group type */
#define PARAM_AFG_CAP      VERB_DATA(0x08) /* audio function group capabilities */
#define PARAM_AW_CAP       VERB_DATA(0x09) /* audio widget capabilities */
#define PARAM_SUP_PCM_SR   VERB_DATA(0x0A) /* supported PCM size, rates */
#define PARAM_SUP_STR_FMT  VERB_DATA(0x0B) /* supported stream formats */
#define PARAM_PIN_CAP      VERB_DATA(0x0C) /* pin capabilities */
#define PARAM_IN_AMP_CAP   VERB_DATA(0x0D) /* input amplifier capabilities */
#define PARAM_CONN_LL      VERB_DATA(0x0E) /* connection list length */
#define PARAM_SUP_PWR_ST   VERB_DATA(0x0F) /* supported power states */
#define PARAM_PROC_CAP     VERB_DATA(0x10) /* processing capabilities */
#define PARAM_GPIO_COUNT   VERB_DATA(0x11) /* GPIO count */
#define PARAM_OUT_AMP_CAP  VERB_DATA(0x12) /* output amplifier capabilities */
#define PARAM_VOL_KNOB_CAP VERB_DATA(0x13) /* volume knob capabilities */

#define FN_GROUP_AFG 1 /* audio function group */
#define FN_GROUP_VDM 2 /* vendor defined modem function group */

#define AFG_CAP_OUTPUT_DELAY(v) (((v) >> 0) & 0xF) /* output delay in samples count */
#define AFG_CAP_INPUT_DELAY(v)  (((v) >> 8) & 0xF) /* input delay in samples count */
#define AFG_CAP_BEEP_GEN        (1 << 16)          /* is beep generation capable */

#define AW_CAP_STEREO    (1 << 0)  /* is stereo */
#define AW_CAP_IN_AMP    (1 << 1)  /* has in amp */
#define AW_CAP_OUT_AMP   (1 << 2)  /* has out amp */
#define AW_CAP_AMP_OR    (1 << 3)  /* amp parameter override */
#define AW_CAP_FMT_OR    (1 << 4)  /* format override */
#define AW_CAP_STRIPE    (1 << 5)  /* stripe */
#define AW_CAP_PROC      (1 << 6)  /* is processing control */
#define AW_CAP_UNSOL     (1 << 7)  /* is unsolicited capable */
#define AW_CAP_CONN_LST  (1 << 8)  /* has connection list */
#define AW_CAP_DIGITAL   (1 << 9)  /* is digital capable */
#define AW_CAP_PWR_CTL   (1 << 10) /* has power control */
#define AW_CAP_LR_SWAP   (1 << 11) /* support left-right swap */
#define AW_CAP_CP_CAPS   (1 << 12) /* supports content protection */
#define AW_CAP_CH_CNT(n) (((n) >> 13) & 0x7) /* channels count */
#define AW_CAP_DELAY(n)  (((n) >> 16) & 0xF) /* delay in samples count */
#define AW_CAP_TYPE(n)   (((n) >> 20) & 0xF) /* widget type */

#define AW_TYPE_OUT      0x0
#define AW_TYPE_IN       0x1
#define AW_TYPE_MIXER    0x2
#define AW_TYPE_SELECT   0x3
#define AW_TYPE_PIN      0x4
#define AW_TYPE_POWER    0x5
#define AW_TYPE_VOL_KNOB 0x6
#define AW_TYPE_BEEP_GEN 0x7
#define AW_TYPE_VENDOR   0xF

#define DEPTH_32 (1 << 20) /* 32 bits */
#define DEPTH_24 (1 << 19) /* 24 bits */
#define DEPTH_20 (1 << 18) /* 20 bits */
#define DEPTH_16 (1 << 17) /* 16 bits */
#define DEPTH_8  (1 << 16) /* 8 bits */

#define RATE_8   (1 << 0)  /* 8.0kHz */
#define RATE_11  (1 << 1)  /* 11.025 kHz */
#define RATE_16  (1 << 2)  /* 16.0 kHz */
#define RATE_22  (1 << 3)  /* 22.05 kHz */
#define RATE_32  (1 << 4)  /* 32.0 kHz */
#define RATE_44  (1 << 5)  /* 44.1 kHz */
#define RATE_48  (1 << 6)  /* 48.0 kHz */
#define RATE_88  (1 << 7)  /* 88.2 kHz */
#define RATE_96  (1 << 8)  /* 96.0 kHz */
#define RATE_176 (1 << 9)  /* 176.4 kHz */
#define RATE_192 (1 << 10) /* 192.0 kHz */
#define RATE_384 (1 << 11) /* 384 kHz */

#define FORMAT_PCM (1 << 0) /* PCM, specified by depth & rate */
#define FORMAT_F32 (1 << 1) /* 32bits float */
#define FORMAT_AC3 (1 << 2) /* dolby AC-3 */

#define PIN_CAP_ISC     (1 << 0) /* impedance sense capable */
#define PIN_CAP_TR      (1 << 1) /* trigger req'd */
#define PIN_CAP_PDC     (1 << 2) /* presence detect capable */
#define PIN_CAP_HDC     (1 << 3) /* headphone driver capable */
#define PIN_CAP_OUT     (1 << 4) /* output capable */
#define PIN_CAP_IN      (1 << 5) /* input capable */
#define PIN_CAP_BIOP    (1 << 6) /* balanced io pins */
#define PIN_CAP_HDMI    (1 << 7) /* HCMI sink */
#define PIN_CAP_VREF(n) (((n) >> 8) & 0xFF) /* vref control */
#define PIN_CAP_EAPD    (1 << 16) /* eapd capable */
#define PIN_CAP_DP      (1 << 24) /* display port sink */
#define PIN_CAP_HBR     (1 << 27) /* high bit rate capable */

#define CONF_DFL_PC(v)   (((v) >> 30) & 0x03) /* port connectivity */
#define CONF_DFL_LOC(v)  (((v) >> 24) & 0x3F) /* location */
#define CONF_DFL_DEV(v)  (((v) >> 20) & 0x0F) /* default device */
#define CONF_DFL_TYPE(v) (((v) >> 16) & 0x0F) /* connection type */
#define CONF_DFL_COL(v)  (((v) >> 12) & 0x0F) /* color */
#define CONF_DFL_MISC(v) (((v) >>  8) & 0x0F) /* misc */
#define CONF_DFL_DA(v)   (((v) >>  4) & 0x0F) /* default association */
#define CONF_DFL_SEQ(v)  (((v) >>  0) & 0x0F) /* sequence */

#define CONF_PC_JACK  0x0 /* jack port connection */
#define CONF_PC_NONE  0x1 /* no port connection */
#define CONF_PC_FIXED 0x2 /* fixed function device */
#define CONF_PC_BOTH  0x3 /* both jack & fixed */

#define CONF_DEV_LINE_OUT    0x0
#define CONF_DEV_SPEAKER     0x1
#define CONF_DEV_HP_OUT      0x2
#define CONF_DEV_CD          0x3
#define CONF_DEV_SPDIF_OUT   0x4
#define CONF_DEV_DIGITAL_OUT 0x5
#define CONF_DEV_MODEM_LINE  0x6
#define CONF_DEV_MODEM_HS    0x7
#define CONF_DEV_LINE_IN     0x8
#define CONF_DEV_AUX         0x9
#define CONF_DEV_MIC_IN      0xA
#define CONF_DEV_TELEPHONY   0xB
#define CONF_DEV_SPDIF_IN    0xC
#define CONF_DEV_DIGITAL_IN  0xD
#define CONF_DEV_RESERVED    0xE
#define CONF_DEV_OTHER       0xF

#define CONF_TYPE_UNK     0x0 /* unknown */
#define CONF_TYPE_1_8     0x1 /* 1/8" stereo/mono */
#define CONF_TYPE_1_4     0x2 /* 1/4" stereo/mono */
#define CONF_TYPE_ATAPI   0x3 /* ATAPI internal */
#define CONF_TYPE_RCA     0x4 /* RCA */
#define CONF_TYPE_OPTICAL 0x5 /* optical */
#define CONF_TYPE_DIGITAL 0x6 /* other digital */
#define CONF_TYPE_ANALOG  0x7 /* other analog */
#define CONF_TYPE_DIN     0x8 /* multichannel analog (DIN) */
#define CONF_TYPE_XLR     0x9 /* XLR / professional */
#define CONF_TYPE_RJ_11   0xA /* RJ-11 (modem) */
#define CONF_TYPE_COMB    0xB /* combination */
#define CONF_TYPE_OTHER   0xF /* other */

#define CONF_COL_UNKNOWN 0x0
#define CONF_COL_BLACK   0x1
#define CONF_COL_GREY    0x2
#define CONF_COL_BLUE    0x3
#define CONF_COL_GREEN   0x4
#define CONF_COL_RED     0x5
#define CONF_COL_ORANGE  0x6
#define CONF_COL_YELLOW  0x7
#define CONF_COL_PURPLE  0x8
#define CONF_COL_PINK    0x9
#define CONF_COL_WHITE   0xE
#define CONF_COL_OTHER   0xF

#define MUTE_SET_GAIN(v)  ((v) & 0x7F)       /* gain */
#define MUTE_SET_MUTE     (1 << 7)           /* mute channel */
#define MUTE_SET_INDEX(v) (((v) & 0xF) << 8) /* input index */
#define MUTE_SET_RAMP     (1 << 12)          /* affect right amplifier */
#define MUTE_SET_LAMP     (1 << 13)          /* affect left amplifier */
#define MUTE_SET_IN_AMP   (1 << 14)          /* affect input amplifier */
#define MUTE_SET_OUT_AMP  (1 << 15)          /* affect output amplifier */

#define AMP_OFFSET(v) (((v) >>  0) & 0x7F) /* offset */
#define AMP_NSTEPS(v) (((v) >>  8) & 0x7F) /* steps count */
#define AMP_STEPSZ(v) (((v) >> 16) & 0x7F) /* step size */
#define AMP_MUTE      (1 << 31)            /* mute capable */

#define CONV_STREAM(n) (((n) & 0xF) << 4) /* stream id */
#define CONV_CHAN(n)   (((n) & 0xF) << 0) /* channel id */

#define POWER_D0        (1 << 0)  /* support d0 power state */
#define POWER_D1        (1 << 1)  /* support d1 power state */
#define POWER_D2        (1 << 2)  /* support d2 power state */
#define POWER_D3        (1 << 3)  /* support d3 power state */
#define POWER_D3_COLD   (1 << 4)  /* support d3 cold power state */
#define POWER_S3D3_COLD (1 << 29) /* support d3 cold power state */
#define POWER_CLKSTOP   (1 << 30) /* support d3 when no BCLK */
#define POWER_EPSS      (1 << 31) /* support low-power operations ? */

#define IRS_ICB (1 << 0) /* immediate command busy */
#define IRS_IRV (1 << 1) /* immediate result valid */

#define CORB_RST (1 << 15) /* corb reset */

#define CORB_CTL_MEIE (1 << 0) /* memory error interrupt enable */
#define CORB_CTL_DMA  (1 << 1)  /* enable / disable DMA */

#define RIRB_RST (1 << 15) /* rirb reset */

#define RIRB_CTL_RIC  (1 << 0) /* response interrupt control */
#define RIRB_CTL_DMA  (1 << 1) /* enable / disable DMA */
#define RIRB_CTL_ROIC (1 << 2) /* response overrun interrupt control */

#define RIRB_STS_RI   (1 << 0) /* rirb response interrupt */
#define RIRB_STS_ROIS (1 << 2) /* rirb response overrun interrupt status */

#define DESC_AHEAD 2

struct buf_desc
{
	uint64_t addr;
	uint32_t size;
	uint32_t ioc;
};

struct hda_widget
{
	uint8_t nid;
	uint32_t capabilities;
	union
	{
		struct
		{
			uint32_t formats;
			uint32_t rates;
			uint32_t amp_cap;
		} out;
		struct
		{
			uint32_t formats;
			uint32_t rates;
			uint32_t amp_cap;
		} in;
		struct
		{
			uint32_t pin_cap;
			uint32_t conf_dfl;
			uint32_t conn_list_len;
			uint8_t *conn_list;
			uint32_t in_amp_cap;
			uint32_t out_amp_cap;
		} pin;
		struct
		{
			uint32_t conn_list_len;
			uint8_t *conn_list;
			uint32_t in_amp_cap;
			uint32_t out_amp_cap;
		} mixer;
	};
};

struct hda_group
{
	uint8_t nid;
	uint8_t widgets_count;
	uint8_t widgets_start;
	uint32_t group_type;
	struct hda_widget *widgets;
	union
	{
		struct
		{
			uint32_t cap;
			uint32_t formats;
			uint32_t rates;
			uint32_t in_amp_cap;
			uint32_t out_amp_cap;
			uint32_t power_states;
			uint32_t subsystem;
		} afg;
	};
};

struct hda_codec
{
	uint8_t cid;
	uint16_t vendor;
	uint16_t device;
	uint32_t revision;
	uint8_t groups_count;
	uint8_t groups_start;
	struct hda_group *groups;
};

struct hda
{
	struct pci_device *device;
	struct pci_map *pci_map;
	struct dma_buf *out_desc;
	struct dma_buf *in_desc;
	struct dma_buf *corb;
	uint16_t corb_size;
	uint16_t corb_wp;
	struct dma_buf *rirb;
	uint16_t rirb_size;
	uint16_t rirb_rp;
	uint16_t cur_desc;
	uint8_t codecs_count;
	struct hda_codec *codecs;
	struct dma_buf *dmap;
	struct irq_handle irq_handle;
	struct snd *out;
	struct snd *in;
	struct node *sysfs_node;
};

static inline uint8_t hda_r8(struct hda *hda, uint32_t reg)
{
	return pci_r8(hda->pci_map, reg);
}

static inline uint16_t hda_r16(struct hda *hda, uint32_t reg)
{
	return pci_r16(hda->pci_map, reg);
}

static inline uint32_t hda_r32(struct hda *hda, uint32_t reg)
{
	return pci_r32(hda->pci_map, reg);
}

static inline void hda_w8(struct hda *hda, uint32_t reg, uint8_t val)
{
	pci_w8(hda->pci_map, reg, val);
}

static inline void hda_w16(struct hda *hda, uint32_t reg, uint16_t val)
{
	pci_w16(hda->pci_map, reg, val);
}

static inline void hda_w32(struct hda *hda, uint32_t reg, uint32_t val)
{
	pci_w32(hda->pci_map, reg, val);
}

static void int_handler(void *userdata)
{
	struct hda *hda = userdata;
	uint32_t intsts;

	intsts = hda_r32(hda, REG_INTSTS);
	if (!intsts)
		return;
#if 0
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	TRACE("[%lu.%03lu] hda int: 0x%08" PRIx32, ts.tv_sec, ts.tv_nsec / 1000000, intsts);
#endif
	for (size_t i = 0; i < 30; ++i)
	{
		if (!(intsts & (1 << i)))
			continue;
		uint8_t sdsts = hda_r8(hda, REG_SDSTS(i));
		hda_w8(hda, REG_SDSTS(i), sdsts);
#if 0
		TRACE("sdsts[%u] = %" PRIx8, (unsigned)i, sdsts);
#endif
#if 0
		TRACE("dmap: 0x%08" PRIx32, ((uint32_t*)hda->dmap->data)[8]);
#endif
		uint32_t pib = hda_r32(hda, REG_SDLPIB(4));
#if 0
		TRACE("pib: 0x%08" PRIx32, pib);
#endif
		uint32_t buf_to = pib / PAGE_SIZE;
		buf_to = (buf_to + DESC_AHEAD) % hda->out->nbufs;
		while (hda->cur_desc != buf_to)
		{
#if 1
			snd_fill_buf(hda->out, hda->out->bufs[hda->cur_desc]);
#endif
			hda->cur_desc = (hda->cur_desc + 1) % hda->out->nbufs;
		}
	}
}

static int setup_desc(struct hda *hda)
{
	int ret = dma_buf_alloc(PAGE_SIZE, 0, &hda->out_desc);
	if (ret)
	{
		TRACE("hda: output descriptor allocation failed");
		return ret;
	}
	memset(hda->out_desc->data, 0, PAGE_SIZE);
	for (size_t i = 0; i < hda->out->nbufs; ++i)
	{
		struct buf_desc *desc = &((struct buf_desc*)hda->out_desc->data)[i];
		desc->addr = pm_page_addr(hda->out->bufs[i]->pages);
		desc->size = PAGE_SIZE;
		desc->ioc = 1;
	}
	return 0;
}

static int setup_corb(struct hda *hda)
{
	uint8_t corbszcap = hda_r8(hda, REG_CORBSIZE) >> 4;
	if (corbszcap & (1 << 2))
	{
		hda->corb_size = 256;
		hda_w8(hda, REG_CORBSIZE, 0x2);
	}
	else if (corbszcap & (1 << 1))
	{
		hda->corb_size = 16;
		hda_w8(hda, REG_CORBSIZE, 0x1);
	}
	else if (corbszcap & (1 << 0))
	{
		hda->corb_size = 2;
		hda_w8(hda, REG_CORBSIZE, 0x0);
	}
	else
	{
		TRACE("hda: invalid corb size cap");
		return -EINVAL;
	}
#if 0
	TRACE("corb size: %" PRIu16, hda->corb_size);
#endif
	int ret = dma_buf_alloc(PAGE_SIZE, 0, &hda->corb);
	if (ret)
	{
		TRACE("hda: corb allocation failed");
		return ret;
	}
	hda_w8(hda, REG_CORBCTL, 0);
	while (hda_r8(hda, REG_CORBCTL) & CORB_CTL_DMA) /* XXX timeout */
		;
	uintptr_t page_addr = pm_page_addr(hda->corb->pages);
	hda_w32(hda, REG_CORBLBASE, page_addr);
	hda_w32(hda, REG_CORBUBASE, sizeof(page_addr) > 4 ? (page_addr >> 32) : 0);
	hda_w16(hda, REG_CORBWP, 0);
	hda_w16(hda, REG_CORBRP, CORB_RST);
	while (!(hda_r16(hda, REG_CORBRP) & CORB_RST)) /* XXX timeout */
		;
	hda_w16(hda, REG_CORBRP, 0);
	while (hda_r16(hda, REG_CORBRP) & CORB_RST) /* XXX timeout */
		;
	hda_w8(hda, REG_CORBCTL, CORB_CTL_DMA | CORB_CTL_MEIE);
	while (!(hda_r8(hda, REG_CORBCTL) & CORB_CTL_DMA)) /* XXX timeout */
		;
	return 0;
}

static int setup_rirb(struct hda *hda)
{
	uint8_t rirbszcap = hda_r8(hda, REG_RIRBSIZE) >> 4;
	if (rirbszcap & (1 << 2))
	{
		hda->rirb_size = 256;
		hda_w8(hda, REG_RIRBSIZE, 0x2);
	}
	else if (rirbszcap & (1 << 1))
	{
		hda->rirb_size = 16;
		hda_w8(hda, REG_RIRBSIZE, 0x1);
	}
	else if (rirbszcap & (1 << 0))
	{
		hda->rirb_size = 2;
		hda_w8(hda, REG_RIRBSIZE, 0x0);
	}
	else
	{
		TRACE("hda: invalid rirb size cap");
		return -EINVAL;
	}
#if 0
	TRACE("rirb size: %" PRIu16, hda->rirb_size);
#endif
	int ret = dma_buf_alloc(PAGE_SIZE, 0, &hda->rirb);
	if (ret)
	{
		TRACE("hda: rirb allocation failed");
		return ret;
	}
	hda_w8(hda, REG_RIRBCTL, 0);
	while (hda_r8(hda, REG_RIRBCTL) & RIRB_CTL_DMA) /* XXX timeout */
		;
	uintptr_t page_addr = pm_page_addr(hda->rirb->pages);
	hda_w32(hda, REG_RIRBLBASE, page_addr);
	hda_w32(hda, REG_RIRBUBASE, sizeof(page_addr) > 4 ? (page_addr >> 32) : 0);
	hda_w16(hda, REG_RINTCNT, 1);
	hda_w16(hda, REG_RIRBWP, RIRB_RST);
	hda_w8(hda, REG_RIRBCTL, RIRB_CTL_DMA | RIRB_CTL_ROIC | RIRB_CTL_RIC);
	while (!(hda_r8(hda, REG_RIRBCTL) & RIRB_CTL_DMA)) /* XXX timeout */
		;
	return 0;
}

static int run_verb(struct hda *hda, uint32_t verb, uint32_t *answer)
{
	static const struct timespec delay = {0, 100};
	hda->corb_wp = (hda->corb_wp + 1) % hda->corb_size;
	((uint32_t*)hda->corb->data)[hda->corb_wp] = verb;
	hda_w16(hda, REG_CORBWP, hda->corb_wp);

again:
	size_t i = 0;
	while (!(hda_r8(hda, REG_RIRBSTS) & RIRB_STS_RI))
	{
		/* about 10 us */
		if (++i > 100)
		{
			TRACE("hda: verb setup timeout");
			return -EINVAL;
		}
		spinsleep(&delay);
	}
	hda_w8(hda, REG_RIRBSTS, RIRB_STS_RI);
	hda->rirb_rp = (hda->rirb_rp + 1) % hda->rirb_size;
	if (((uint32_t*)hda->rirb->data)[hda->rirb_rp * 2 + 1] & (1 << 4))
	{
		TRACE("hda: unsolicited response");
		goto again;
	}
	if (answer)
		*answer = ((uint32_t*)hda->rirb->data)[hda->rirb_rp * 2];
	return 0;
}

static inline int widget_verb(struct hda *hda, struct hda_codec *codec,
                              struct hda_widget *widget, uint32_t verb,
                             uint32_t *answer)
{
	return run_verb(hda, VERB_CODEC(codec->cid)
	                   | VERB_NODE(widget->nid)
	                   | verb,
	                     answer);
}

static inline int widget_param(struct hda *hda, struct hda_codec *codec,
                               struct hda_widget *widget, uint32_t param,
                               uint32_t *answer)
{
	return widget_verb(hda, codec, widget, VERB_CMD_GET_PARAM | param,
	                   answer);
}

static inline int group_verb(struct hda *hda, struct hda_codec *codec,
                             struct hda_group *group, uint32_t verb,
                             uint32_t *answer)
{
	return run_verb(hda, VERB_CODEC(codec->cid)
	                   | VERB_NODE(group->nid)
	                   | verb,
	                     answer);
}

static inline int group_param(struct hda *hda, struct hda_codec *codec,
                              struct hda_group *group, uint32_t param,
                              uint32_t *answer)
{
	return group_verb(hda, codec, group, VERB_CMD_GET_PARAM | param,
	                  answer);
}

static inline int root_verb(struct hda *hda, struct hda_codec *codec,
                            uint32_t verb, uint32_t *answer)
{
	return run_verb(hda, VERB_CODEC(codec->cid)
	                   | VERB_NODE(0)
	                   | verb,
	                     answer);
}

static inline int root_param(struct hda *hda, struct hda_codec *codec,
                             uint32_t param, uint32_t *answer)
{
	return root_verb(hda, codec, VERB_CMD_GET_PARAM | param, answer);
}

static int parse_conn_list(struct hda *hda, struct hda_codec *codec,
                           struct hda_widget *widget,
                           uint8_t **conn_list, uint32_t *conn_list_len)
{
	int ret = widget_param(hda, codec, widget, PARAM_CONN_LL, conn_list_len);
	if (ret)
		return ret;
	*conn_list = malloc(sizeof(*conn_list) * (*conn_list_len + 1), M_ZERO);
	if (!*conn_list)
	{
		TRACE("hda: failed to allocate pin connection list");
		return -ENOMEM;
	}
	for (uint32_t i = 0; i < *conn_list_len;)
	{
		ret = widget_verb(hda, codec, widget,
		                  VERB_CMD_SET_CSC | VERB_DATA(i),
		                  NULL);
		if (ret)
			return ret;
		uint32_t tmp;
		ret = widget_verb(hda, codec, widget,
		                  VERB_CMD_GET_CLEC,
		                  &tmp);
		if (ret)
			return ret;
		size_t n = *conn_list_len - i;
		if (n > 4)
			 n = 4;
		for (size_t j = 0; j < n; ++j)
		{
			(*conn_list)[i++] = tmp & 0xFF;
			tmp >>= 8;
		}
	}
	return 0;
}

static int parse_widget(struct hda *hda, struct hda_codec *codec,
                        struct hda_widget *widget)
{
	uint32_t aw_cap;
	int ret = widget_param(hda, codec, widget, PARAM_AW_CAP, &aw_cap);
	if (ret)
		return ret;
	widget->capabilities = aw_cap;
	switch (AW_CAP_TYPE(aw_cap))
	{
		case AW_TYPE_OUT:
			ret = widget_param(hda, codec, widget, PARAM_SUP_PCM_SR,
			                  &widget->out.rates);
			if (ret)
				return ret;
			ret = widget_param(hda, codec, widget, PARAM_SUP_STR_FMT,
			                   &widget->out.formats);
			if (ret)
				return ret;
			ret = widget_param(hda, codec, widget, PARAM_OUT_AMP_CAP,
			                  &widget->out.amp_cap);
			if (ret)
				return ret;
			break;
		case AW_TYPE_IN:
			ret = widget_param(hda, codec, widget, PARAM_SUP_PCM_SR,
			                   &widget->in.rates);
			if (ret)
				return ret;
			ret = widget_param(hda, codec, widget, PARAM_SUP_STR_FMT,
			                   &widget->in.formats);
			if (ret)
				return ret;
			ret = widget_param(hda, codec, widget, PARAM_IN_AMP_CAP,
			                   &widget->in.amp_cap);
			if (ret)
				return ret;
			break;
		case AW_TYPE_MIXER:
			ret = parse_conn_list(hda, codec, widget,
			                      &widget->mixer.conn_list,
			                      &widget->mixer.conn_list_len);
			if (ret)
				return ret;
			ret = widget_param(hda, codec, widget, PARAM_IN_AMP_CAP,
			                   &widget->mixer.in_amp_cap);
			if (ret)
				return ret;
			ret = widget_param(hda, codec, widget, PARAM_OUT_AMP_CAP,
			                   &widget->mixer.out_amp_cap);
			if (ret)
				return ret;
			break;
		case AW_TYPE_SELECT:
			break;
		case AW_TYPE_PIN:
			ret = widget_param(hda, codec, widget, PARAM_PIN_CAP,
			                   &widget->pin.pin_cap);
			if (ret)
				return ret;
			ret = widget_verb(hda, codec, widget,
			                  VERB_CMD_GET_CONF_DFL,
			                  &widget->pin.conf_dfl);
			if (ret)
				return ret;
			ret = parse_conn_list(hda, codec, widget,
			                      &widget->pin.conn_list,
			                      &widget->pin.conn_list_len);
			if (ret)
				return ret;
			ret = widget_param(hda, codec, widget, PARAM_IN_AMP_CAP,
			                   &widget->pin.in_amp_cap);
			if (ret)
				return ret;
			ret = widget_param(hda, codec, widget, PARAM_OUT_AMP_CAP,
			                   &widget->pin.out_amp_cap);
			if (ret)
				return ret;
			break;
		case AW_TYPE_POWER:
			break;
		case AW_TYPE_VOL_KNOB:
			break;
		case AW_TYPE_BEEP_GEN:
			break;
		case AW_TYPE_VENDOR:
			break;
		default:
			TRACE("hda: unknown widget type");
			return 0;
	}
	return 0;
}

static int parse_group(struct hda *hda, struct hda_codec *codec,
                       struct hda_group *group)
{
	uint32_t nodes;
	uint32_t group_type;
	int ret;
	ret = group_param(hda, codec, group, PARAM_SUB_NODE_NB, &nodes);
	if (ret)
		return ret;
	ret = group_param(hda, codec, group, PARAM_FN_GRP_TYPE, &group_type);
	if (ret)
		return ret;
	group->widgets_count = (nodes >>  0) & 0xFF;
	group->widgets_start = (nodes >> 16) & 0xFF;
	group->group_type = group_type;
	switch (group->group_type & 0xF)
	{
		case FN_GROUP_AFG:
			ret = group_param(hda, codec, group, PARAM_AFG_CAP,
			                  &group->afg.cap);
			if (ret)
				return ret;
			ret = group_param(hda, codec, group, PARAM_SUP_PCM_SR,
			                  &group->afg.rates);
			if (ret)
				return ret;
			ret = group_param(hda, codec, group, PARAM_SUP_STR_FMT,
			                  &group->afg.formats);
			if (ret)
				return ret;
			ret = group_param(hda, codec, group, PARAM_IN_AMP_CAP,
			                  &group->afg.in_amp_cap);
			if (ret)
				return ret;
			ret = group_param(hda, codec, group, PARAM_OUT_AMP_CAP,
			                  &group->afg.out_amp_cap);
			if (ret)
				return ret;
			ret = group_param(hda, codec, group, PARAM_SUP_PWR_ST,
			                  &group->afg.power_states);
			if (ret)
				return ret;
			ret = group_verb(hda, codec, group, VERB_CMD_GET_IMP_ID,
			                  &group->afg.subsystem);
			if (ret)
				return ret;
			break;
		default:
			TRACE("hda: unknown group type: %" PRIx32,
			      group->group_type & 0xF);
			return 0;
	}
	if (!group->widgets_count)
		return 0;
	group->widgets = malloc(sizeof(*group->widgets) * group->widgets_count, M_ZERO);
	if (!group->widgets)
	{
		TRACE("hda: failed to allocate group widgets");
		return -ENOMEM;
	}
	for (uint16_t i = 0; i < group->widgets_count; ++i)
	{
		group->widgets[i].nid = group->widgets_start + i;
		ret = parse_widget(hda, codec, &group->widgets[i]);
		if (ret)
			return ret;
	}
	return 0;
}

static int parse_codec(struct hda *hda, struct hda_codec *codec)
{
	uint32_t vendor;
	uint32_t revision;
	uint32_t nodes;
	int ret;
	ret = root_param(hda, codec, PARAM_VENDOR_ID, &vendor);
	if (ret)
		return ret;
	ret = root_param(hda, codec, PARAM_REVISION_ID, &revision);
	if (ret)
		return ret;
	ret = root_param(hda, codec, PARAM_SUB_NODE_NB, &nodes);
	if (ret)
		return ret;
	codec->vendor = (vendor >> 16) & 0xFFFF;
	codec->device = (vendor >>  0) & 0xFFFF;
	codec->revision = revision;
	codec->groups_count = (nodes >>  0) & 0xFF;
	codec->groups_start = (nodes >> 16) & 0xFF;
	if (!codec->groups_count)
		return 0;
	codec->groups = malloc(sizeof(*codec->groups) * codec->groups_count,
	                       M_ZERO);
	if (!codec->groups)
	{
		TRACE("hda: groups allocation failed");
		return -ENOMEM;
	}
	for (uint16_t i = 0; i < codec->groups_count; ++i)
	{
		codec->groups[i].nid = codec->groups_start + i;
		ret = parse_group(hda, codec, &codec->groups[i]);
		if (ret)
			return ret;
	}
	return 0;
}

static int parse_codecs(struct hda *hda)
{
	hda->codecs_count = 0;
	uint16_t statests = hda_r16(hda, REG_STATESTS);
	hda_w16(hda, REG_STATESTS, statests);
	for (size_t i = 0; i < 15; ++i)
	{
		if (!(statests & (1 << i)))
			continue;
		hda->codecs_count++;
	}
	hda->codecs = malloc(sizeof(*hda->codecs) * hda->codecs_count, M_ZERO);
	if (!hda->codecs)
	{
		TRACE("hda: failed to allocate codecs");
		return -ENOMEM;
	}
	size_t n = 0;
	for (size_t i = 0; i < 15; ++i)
	{
		if (!(statests & (1 << i)))
			continue;
		hda->codecs[n].cid = i;
		int ret = parse_codec(hda, &hda->codecs[n]);
		if (ret)
			return ret;
		n++;
	}
	return 0;
}

#define PRINT_CAP(fmt, ...) \
do \
{ \
	if (first) \
		first = 0; \
	else \
		uprintf(uio, ", "); \
	uprintf(uio, fmt, ##__VA_ARGS__); \
} while (0)

static inline void print_rates(struct uio *uio, uint32_t rates)
{
	int first = 1;

#define TEST_DEPTH(d) \
do \
{ \
	if (!(rates & DEPTH_##d)) \
		break; \
	PRINT_CAP(#d "bits"); \
} while (0)

#define TEST_RATE(r) \
do \
{ \
	if (!(rates & RATE_##r)) \
		break; \
	PRINT_CAP(#r "kHz"); \
} while (0)

	TEST_DEPTH(32);
	TEST_DEPTH(24);
	TEST_DEPTH(20);
	TEST_DEPTH(16);
	TEST_DEPTH(8);
	TEST_RATE(8);
	TEST_RATE(11);
	TEST_RATE(16);
	TEST_RATE(22);
	TEST_RATE(32);
	TEST_RATE(44);
	TEST_RATE(48);
	TEST_RATE(88);
	TEST_RATE(96);
	TEST_RATE(176);
	TEST_RATE(192);
	TEST_RATE(384);

#undef TEST_RATE
#undef TEST_DEPTH
}

static inline void print_formats(struct uio *uio, uint32_t formats)
{
	int first = 1;

#define TEST_FORMAT(f) \
do \
{ \
	if (!(formats & FORMAT_##f)) \
		break; \
	PRINT_CAP(#f); \
} while (0)

	TEST_FORMAT(PCM);
	TEST_FORMAT(F32);
	TEST_FORMAT(AC3);

#undef TEST_FORMAT
}

static inline void print_aw_cap(struct uio *uio, uint32_t cap)
{
	int first = 1;

#define TEST_CAP(c, str) \
do \
{ \
	if (!(cap & c)) \
		break; \
	PRINT_CAP(str); \
} while (0)

	static const char *types[] =
	{
		"output",
		"input",
		"mixer",
		"select",
		"pin",
		"power",
		"volume knob",
		"beep generator",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"vendor",
	};

	TEST_CAP(AW_CAP_STEREO, "stereo");
	TEST_CAP(AW_CAP_IN_AMP, "in amp");
	TEST_CAP(AW_CAP_OUT_AMP, "out amp");
	TEST_CAP(AW_CAP_AMP_OR, "amp override");
	TEST_CAP(AW_CAP_FMT_OR, "format override");
	TEST_CAP(AW_CAP_STRIPE, "stripe");
	TEST_CAP(AW_CAP_PROC, "processing control");
	TEST_CAP(AW_CAP_UNSOL, "unsolicited");
	TEST_CAP(AW_CAP_CONN_LST, "connection list");
	TEST_CAP(AW_CAP_CONN_LST, "digital");
	TEST_CAP(AW_CAP_PWR_CTL, "power control");
	TEST_CAP(AW_CAP_LR_SWAP, "l-r swap");
	TEST_CAP(AW_CAP_CP_CAPS, "content protection");
	PRINT_CAP("channels: %" PRIu32, AW_CAP_CH_CNT(cap));
	PRINT_CAP("delay: %" PRIu32, AW_CAP_DELAY(cap));
	PRINT_CAP("type: %s", types[AW_CAP_TYPE(cap)]);

#undef TEST_CAP
}

static inline void print_afg_cap(struct uio *uio, uint32_t cap)
{
	int first = 1;
	PRINT_CAP("output delay: %" PRIu32, AFG_CAP_OUTPUT_DELAY(cap));
	PRINT_CAP("input delay: %" PRIu32, AFG_CAP_INPUT_DELAY(cap));
	PRINT_CAP("beep: %s", (cap & AFG_CAP_BEEP_GEN) ? "yes" : "no");
}

static inline void print_pin_cap(struct uio *uio, uint32_t cap)
{
	int first = 1;

#define TEST_CAP(c, str) \
do \
{ \
	if (!(cap & c)) \
		break; \
	PRINT_CAP(str); \
} while (0)

	TEST_CAP(PIN_CAP_ISC, "impedance sense");
	TEST_CAP(PIN_CAP_ISC, "trigger req");
	TEST_CAP(PIN_CAP_PDC, "presence detect");
	TEST_CAP(PIN_CAP_HDC, "headphone driver");
	TEST_CAP(PIN_CAP_OUT, "output");
	TEST_CAP(PIN_CAP_IN, "input");
	TEST_CAP(PIN_CAP_BIOP, "balanced io");
	TEST_CAP(PIN_CAP_HDMI, "HDMI");
	TEST_CAP(PIN_CAP_EAPD, "EAPD");
	TEST_CAP(PIN_CAP_DP, "display port");
	TEST_CAP(PIN_CAP_HBR, "high bit rate");
	PRINT_CAP("vref: 0x%02" PRIx32, PIN_CAP_VREF(cap));

#undef TEST_CAP
}

static inline void print_conf_dfl(struct uio *uio, uint32_t dfl)
{
	int first = 1;
	static const char *dfl_port_conns[] =
	{
		"jack",
		"none",
		"fixed",
		"both",
	};
	static const char *dfl_devices[] =
	{
		"line out",
		"speaker",
		"headphone out",
		"CD",
		"S/PDIF out",
		"digital out",
		"modem line",
		"modem handset",
		"line in",
		"AUX",
		"mic in",
		"telephony",
		"S/PDIF in",
		"digital in",
		"reserved",
		"other",
	};
	static const char *dfl_types[] =
	{
		"unknown",
		"1/8\" stereo/mono",
		"1/4\" stereo/mono",
		"ATAPI internal",
		"RCA",
		"optical",
		"other digital",
		"other analog",
		"multichannel analog",
		"XLR",
		"RJ-11",
		"combination",
		"unknown",
		"unknown",
		"unknown",
		"other",
	};
	static const char *dfl_colors[] =
	{
		"unknown",
		"black",
		"grey",
		"blue",
		"green",
		"red",
		"orange",
		"yellow",
		"purple",
		"pink",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"white",
		"other",
	};
	PRINT_CAP("port conn: %s", dfl_port_conns[CONF_DFL_PC(dfl)]);
	PRINT_CAP("location: 0x%" PRIx32, CONF_DFL_LOC(dfl));
	PRINT_CAP("device: %s", dfl_devices[CONF_DFL_DEV(dfl)]);
	PRINT_CAP("type: %s", dfl_types[CONF_DFL_TYPE(dfl)]);
	PRINT_CAP("color: %s", dfl_colors[CONF_DFL_COL(dfl)]);
	PRINT_CAP("misc: 0x%" PRIx32, CONF_DFL_MISC(dfl));
	PRINT_CAP("default assoc: 0x%" PRIx32, CONF_DFL_DA(dfl));
	PRINT_CAP("sequence: 0x%" PRIx32, CONF_DFL_SEQ(dfl));
}

static inline void print_amp_cap(struct uio *uio, uint32_t cap)
{
	int first = 1;
	PRINT_CAP("offset: 0x%" PRIx32, AMP_OFFSET(cap));
	PRINT_CAP("nsteps: %" PRIu32, AMP_NSTEPS(cap));
	PRINT_CAP("step size: %" PRIu32, AMP_STEPSZ(cap));
	PRINT_CAP("mute: %s", (cap & AMP_MUTE) ? "yes" : "no");
}

static inline void print_power_states(struct uio *uio, uint32_t cap)
{
	int first = 1;

#define TEST_CAP(c, str) \
do \
{ \
	if (!(cap & POWER_##c)) \
		break; \
	PRINT_CAP(str); \
} while (0)

	TEST_CAP(D0, "D0");
	TEST_CAP(D1, "D1");
	TEST_CAP(D2, "D2");
	TEST_CAP(D3, "D3");
	TEST_CAP(D3_COLD, "D3 cold");
	TEST_CAP(S3D3_COLD, "S3D3");
	TEST_CAP(CLKSTOP, "CLKSTOP");
	TEST_CAP(EPSS, "EPSS");

#undef TEST_CAP
}

#undef PRINT_CAP

static const char *widget_type_name(const struct hda_widget *widget)
{
	switch (AW_CAP_TYPE(widget->capabilities))
	{
		case AW_TYPE_OUT:
			return "out";
		case AW_TYPE_IN:
			return "in";
		case AW_TYPE_MIXER:
			return "mixer";
		case AW_TYPE_PIN:
			return "pin";
		case AW_TYPE_POWER:
			return "power";
		case AW_TYPE_VOL_KNOB:
			return "volume knob";
		case AW_TYPE_BEEP_GEN:
			return "beep generator";
		case AW_TYPE_VENDOR:
			return "vendor";
	}
	return "unknown";
}

static inline void print_widget(struct uio *uio,
                                const struct hda_widget *widget)
{
	uprintf(uio, "    widget 0x%02" PRIx8 ": %s\n", widget->nid, widget_type_name(widget));
	uprintf(uio, "      capabilities: 0x%08" PRIx32 "(", widget->capabilities);
	print_aw_cap(uio, widget->capabilities);
	uprintf(uio, ")\n");
	switch (AW_CAP_TYPE(widget->capabilities))
	{
		case AW_TYPE_OUT:
			uprintf(uio, "      out rates: 0x%08" PRIx32 " (",
			        widget->out.rates);
			print_rates(uio, widget->out.rates);
			uprintf(uio, ")\n");
			uprintf(uio, "      out formats: 0x%08" PRIx32 " (",
			        widget->out.formats);
			print_formats(uio, widget->out.formats);
			uprintf(uio, ")\n");
			uprintf(uio, "      out amp cap: 0x%08" PRIx32 " (",
			        widget->out.amp_cap);
			print_amp_cap(uio, widget->out.amp_cap);
			uprintf(uio, ")\n");
			break;
		case AW_TYPE_IN:
			uprintf(uio, "      in rates: 0x%08" PRIx32 " (",
			       widget->in.rates);
			print_rates(uio, widget->in.rates);
			uprintf(uio, ")\n");
			uprintf(uio, "      in formats: 0x%08" PRIx32 " (",
			        widget->in.formats);
			print_formats(uio, widget->in.formats);
			uprintf(uio, ")\n");
			uprintf(uio, "      in amp cap: 0x%08" PRIx32 " (",
			       widget->in.amp_cap);
			print_amp_cap(uio, widget->in.amp_cap);
			uprintf(uio, ")\n");
			break;
		case AW_TYPE_MIXER:
			uprintf(uio, "      mixer in amp cap: 0x%08" PRIx32 " (",
			        widget->mixer.in_amp_cap);
			print_amp_cap(uio, widget->mixer.in_amp_cap);
			uprintf(uio, ")\n");
			uprintf(uio, "      mixer out amp cap: 0x%08" PRIx32 " (",
			        widget->mixer.out_amp_cap);
			print_amp_cap(uio, widget->mixer.out_amp_cap);
			uprintf(uio, ")\n");
			uprintf(uio, "      mixer conn list:");
			for (uint32_t i = 0; i < widget->mixer.conn_list_len; ++i)
				uprintf(uio, " 0x%02" PRIx8, widget->mixer.conn_list[i]);
			uprintf(uio, "\n");
			break;
		case AW_TYPE_PIN:
			uprintf(uio, "      pin capabilities: 0x%08" PRIx32 " (",
			        widget->pin.pin_cap);
			print_pin_cap(uio, widget->pin.pin_cap);
			uprintf(uio, ")\n");
			uprintf(uio, "      pin conf dfl: 0x%08" PRIx32 " (",
			        widget->pin.conf_dfl);
			print_conf_dfl(uio, widget->pin.conf_dfl);
			uprintf(uio, ")\n");
			uprintf(uio, "      pin in amp cap: 0x%08" PRIx32 " (",
			       widget->pin.in_amp_cap);
			print_amp_cap(uio, widget->pin.in_amp_cap);
			uprintf(uio, ")\n");
			uprintf(uio, "      pin out amp cap: 0x%08" PRIx32 " (",
			        widget->pin.out_amp_cap);
			print_amp_cap(uio, widget->pin.out_amp_cap);
			uprintf(uio, ")\n");
			uprintf(uio, "      pin conn list:");
			for (uint32_t i = 0; i < widget->pin.conn_list_len; ++i)
				uprintf(uio, " 0x%02" PRIx8, widget->pin.conn_list[i]);
			uprintf(uio, "\n");
			break;
		default:
			uprintf(uio, "hda: unknown widget type: %" PRIx32 "\n",
			        AW_CAP_TYPE(widget->capabilities));
			break;
	}
}

static inline void print_group(struct uio *uio, const struct hda_group *group,
                               int print_widgets)
{
	uprintf(uio, "  group 0x%02" PRIx8 ":\n", group->nid);
	uprintf(uio, "    widgets: 0x%02" PRIx8 "@0x%02" PRIx8 "\n",
	        group->widgets_count, group->widgets_start);
	uprintf(uio, "    type: 0x%08" PRIx32 "\n", group->group_type);
	switch (group->group_type & 0xF)
	{
		case FN_GROUP_AFG:
			uprintf(uio, "    capabilities: 0x%08" PRIx32 " (",
			        group->afg.cap);
			print_afg_cap(uio, group->afg.cap);
			uprintf(uio, ")\n");
			uprintf(uio, "    rates: 0x%08" PRIx32 " (",
			        group->afg.rates);
			print_rates(uio, group->afg.rates);
			uprintf(uio, ")\n");
			uprintf(uio, "    formats: 0x%08" PRIx32 " (",
			        group->afg.formats);
			print_formats(uio, group->afg.formats);
			uprintf(uio, ")\n");
			uprintf(uio, "    in amp cap: 0x%08" PRIx32 " (",
			        group->afg.in_amp_cap);
			print_amp_cap(uio, group->afg.in_amp_cap);
			uprintf(uio, ")\n");
			uprintf(uio, "    out amp cap: 0x%08" PRIx32 " (",
			        group->afg.out_amp_cap);
			print_amp_cap(uio, group->afg.out_amp_cap);
			uprintf(uio, ")\n");
			uprintf(uio, "    power states: 0x%08" PRIx32 "(",
			        group->afg.power_states);
			print_power_states(uio, group->afg.power_states);
			uprintf(uio, ")\n");
			uprintf(uio, "    subsystem: 0x%08" PRIx32 "\n",
			        group->afg.subsystem);
			break;
		default:
			uprintf(uio, "hda: unknown group type\n");
			break;
	}
	if (print_widgets)
	{
		for (uint16_t i = 0; i < group->widgets_count; ++i)
			print_widget(uio, &group->widgets[i]);
	}
}

static inline void print_codec(struct uio *uio, const struct hda_codec *codec,
                               int print_groups)
{
	uprintf(uio, "codec %" PRIu8 ":\n", codec->cid);
	uprintf(uio, "  vendor: 0x%04" PRIx16 "\n", codec->vendor);
	uprintf(uio, "  device: 0x%04" PRIx16 "\n", codec->device);
	uprintf(uio, "  revision: 0x%08" PRIx32 "\n", codec->revision);
	uprintf(uio, "  groups: 0x%02" PRIx8 "@0x%02" PRIx8 "\n",
	        codec->groups_count, codec->groups_start);
	if (print_groups)
	{
		for (uint16_t i = 0; i < codec->groups_count; ++i)
			print_group(uio, &codec->groups[i], 1);
	}
}

static struct hda_group *get_group(const struct hda_codec *codec, uint8_t nid)
{
	for (uint16_t i = 0; i < codec->groups_count; ++i)
	{
		struct hda_group *group = &codec->groups[i];
		if (group->nid == nid)
			return group;
	}
	return NULL;
}

static struct hda_widget *get_widget(const struct hda_codec *codec, uint8_t nid)
{
	for (uint16_t i = 0; i < codec->groups_count; ++i)
	{
		struct hda_group *group = &codec->groups[i];
		for (uint16_t j = 0; j < group->widgets_count; ++j)
		{
			struct hda_widget *widget = &group->widgets[j];
			if (widget->nid == nid)
				return widget;
		}
	}
	return NULL;
}

static int setup_stream(struct hda *hda)
{
	hda_w16(hda, REG_SDCTL(4), 0);
	while (hda_r16(hda, REG_SDCTL(4)) & SDCTL_RUN) /* XXX timeout */
		;
	hda_w16(hda, REG_SDCTL(4), SDCTL_SRST);
	/* XXX old qemu workaround (ecd5f2882fdd10f798984eb52abd00ffc78c2ef7) */
#if 0
	while (!(hda_w16(hda, REG_SDCTL(4)) & SDCTL_SRST)) /* XXX timeout */
		;
#endif
	hda_w16(hda, REG_SDCTL(4), 0);
	while ((hda_r16(hda, REG_SDCTL(4)) & SDCTL_SRST)) /* XXX tmeout */
		;
	hda_w16(hda, REG_SDFMT(4), SDFMT_STEREO_16_48_PCM);
	hda_w16(hda, REG_SDLVI(4), hda->out->nbufs - 1);
	uintptr_t page_addr = pm_page_addr(hda->out_desc->pages);
	hda_w32(hda, REG_SDBDPL(4), page_addr);
	hda_w32(hda, REG_SDBDPU(4), sizeof(page_addr) > 4 ? (page_addr >> 32) : 0);
	hda_w32(hda, REG_SDCBL(4), hda->out->nbufs * PAGE_SIZE);
	hda_w8(hda, REG_SDID(4), SDID_STRM(1));
	return 0;
}

static int setup_out(struct hda *hda, struct hda_codec *codec,
                     struct hda_widget *out)
{
	int ret;
	ret = widget_verb(hda, codec, out,
	                  VERB_CMD_SET_PWR_ST | VERB_DATA(0), NULL);
	if (ret)
		return ret;
#if 0
	ret = widget_verb(hda, codec, out,
	                  VERB_CMD_SET_AMP_GAIN | VERB_DATA(MUTE_SET_GAIN(0x4A) | MUTE_SET_RAMP | MUTE_SET_LAMP | MUTE_SET_OUT_AMP), NULL);
	if (ret)
		return ret;
#endif
	ret = widget_verb(hda, codec, out,
	                  VERB_CMD_SET_PIN_WC | VERB_DATA(0x40), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, out,
	                  VERB_CMD_SET_EAPD_EN | VERB_DATA(0x2), NULL);
	if (ret)
		return ret;
	return 0;
}

static int setup_dac(struct hda *hda, struct hda_codec *codec,
                     struct hda_widget *dac)
{
	int ret;
	ret = widget_verb(hda, codec, dac,
	                  VERB_CMD_SET_PWR_ST | VERB_DATA(0), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, dac,
	                  VERB_CMD_SET_CONV_CTL | VERB_DATA(CONV_STREAM(1) | CONV_CHAN(0)), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, dac,
	                  VERB_CMD_SET_AMP_GAIN | VERB_DATA(MUTE_SET_GAIN(0x4A) | MUTE_SET_RAMP | MUTE_SET_LAMP | MUTE_SET_OUT_AMP), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, dac,
	                  VERB_CMD_SET_CONV_FMT | VERB_DATA(SDFMT_STEREO_16_48_PCM), NULL);
	if (ret)
		return ret;
	return 0;
}

static int setup_dev_qemu_output(struct hda *hda, struct hda_codec *codec)
{
	struct hda_group *group = get_group(codec, 0x1);
	if (!group)
	{
		TRACE("hda: group not found");
		return  -EINVAL;
	}
	struct hda_widget *dac = get_widget(codec, 0x2);
	if (!dac)
	{
		TRACE("hda: dac widget not found");
		return -EINVAL;
	}
	struct hda_widget *out = get_widget(codec, 0x3);
	if (!out)
	{
		TRACE("hda: out widget not found");
		return -EINVAL;
	}
#if 0
	uprintf(NULL, "group:\n");
	print_group(NULL, group, 0);
	uprintf(NULL, "\n");
	uprintf(NULL, "dac:\n");
	print_widget(NULL, dac);
	uprintf(NULL, "\n");
	uprintf(NULL, "out:\n");
	print_widget(NULL, out);
#endif
	int ret = setup_out(hda, codec, out);
	if (ret)
		return ret;
	ret = setup_dac(hda, codec, dac);
	if (ret)
		return ret;
	return 0;
}

static int setup_dev_qemu_duplex(struct hda *hda, struct hda_codec *codec)
{
	struct hda_group *group = get_group(codec, 0x1);
	if (!group)
	{
		TRACE("hda: group not found");
		return  -EINVAL;
	}
	struct hda_widget *dac = get_widget(codec, 0x2);
	if (!dac)
	{
		TRACE("hda: dac widget not found");
		return -EINVAL;
	}
	struct hda_widget *out = get_widget(codec, 0x3);
	if (!out)
	{
		TRACE("hda: out widget not found");
		return -EINVAL;
	}
	struct hda_widget *adc = get_widget(codec, 0x4);
	if (!adc)
	{
		TRACE("hda: adc widget not found");
		return -EINVAL;
	}
	struct hda_widget *in = get_widget(codec, 0x5);
	if (!in)
	{
		TRACE("hda: in widget not found");
		return -EINVAL;
	}
#if 0
	uprintf(NULL, "group:");
	print_group(NULL, group, 0);
	uprintf(NULL, "\n");
	uprintf(NULL, "dac:");
	print_widget(NULL, dac);
	uprintf(NULL, "\n");
	uprintf(NULL, "out:\n");
	print_widget(NULL, out);
	uprintf(NULL, "\n");
	uprintf(NULL, "adc:\n");
	print_widget(NULL, adc);
	uprintf(NULL, "\n");
	uprintf(NULL, "in:\n");
	print_widget(NULL, in);
#endif
	int ret = setup_out(hda, codec, out);
	if (ret)
		return ret;
	ret = setup_dac(hda, codec, dac);
	if (ret)
		return ret;
	return 0;
}

static int setup_dev_qemu_micro(struct hda *hda, struct hda_codec *codec)
{
	struct hda_group *group = get_group(codec, 0x1);
	if (!group)
	{
		TRACE("hda: group not found");
		return  -EINVAL;
	}
	struct hda_widget *dac = get_widget(codec, 0x2);
	if (!dac)
	{
		TRACE("hda: dac widget not found");
		return -EINVAL;
	}
	struct hda_widget *out = get_widget(codec, 0x3);
	if (!out)
	{
		TRACE("hda: out widget not found");
		return -EINVAL;
	}
	struct hda_widget *adc = get_widget(codec, 0x4);
	if (!adc)
	{
		TRACE("hda: adc widget not found");
		return -EINVAL;
	}
	struct hda_widget *in = get_widget(codec, 0x5);
	if (!in)
	{
		TRACE("hda: in widget not found");
		return -EINVAL;
	}
#if 0
	uprintf(NULL, "group:\n");
	print_group(NULL, group, 0);
	uprintf(NULL, "\n");
	uprintf(NULL, "dac:\n");
	print_widget(NULL, dac);
	uprintf(NULL, "\n");
	uprintf(NULL, "out:\n");
	print_widget(NULL, out);
	uprintf(NULL, "\n");
	uprintf(NULL, "adc:\n");
	print_widget(NULL, adc);
	uprintf(NULL, "\n");
	uprintf(NULL, "in:\n");
	print_widget(NULL, in);
#endif
	int ret = setup_out(hda, codec, out);
	if (ret)
		return ret;
	ret = setup_dac(hda, codec, dac);
	if (ret)
		return ret;
	return 0;
}

static int setup_dev_alc269(struct hda *hda, struct hda_codec *codec)
{
	struct hda_group *group = get_group(codec, 0x1);
	if (!group)
	{
		TRACE("hda: group not found");
		return  -EINVAL;
	}
	struct hda_widget *out = get_widget(codec, 0x14);
	if (!out)
	{
		TRACE("hda: out widget not found");
		return -EINVAL;
	}
	struct hda_widget *mixer = get_widget(codec, 0xC);
	if (!mixer)
	{
		TRACE("hda: mixer widget not found");
		return -EINVAL;
	}
	struct hda_widget *dac = get_widget(codec, 0x2);
	if (!dac)
	{
		TRACE("hda: dac widget not found");
		return -EINVAL;
	}
#if 0
	uprintf(NULL, "group:\n");
	print_group(NULL, group, 0);
	uprintf(NULL, "\n");
	uprintf(NULL, "out:\n");
	print_widget(NULL, out);
	uprintf(NULL, "\n");
	uprintf(NULL, "mixer:\n");
	print_widget(NULL, mixer);
	uprintf(NULL, "\n");
	uprintf(NULL, "dac:\n");
	print_widget(NULL, dac);
#endif
	int ret;
	ret = group_verb(hda, codec, group, VERB_CMD_FN_RESET, NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, out,
	                  VERB_CMD_SET_AMP_GAIN | VERB_DATA(MUTE_SET_RAMP | MUTE_SET_LAMP | MUTE_SET_OUT_AMP), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, out,
	                  VERB_CMD_SET_PIN_WC | VERB_DATA(0x40), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, out,
	                  VERB_CMD_SET_EAPD_EN | VERB_DATA(0x2), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, dac,
	                  VERB_CMD_SET_CONV_CTL | VERB_DATA(CONV_STREAM(1) | CONV_CHAN(0)), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, dac,
	                  VERB_CMD_SET_AMP_GAIN | VERB_DATA(MUTE_SET_GAIN(0x57) | MUTE_SET_RAMP | MUTE_SET_LAMP | MUTE_SET_OUT_AMP), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, dac,
	                  VERB_CMD_SET_CONV_FMT | VERB_DATA(SDFMT_STEREO_16_48_PCM), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, mixer,
	                  VERB_CMD_SET_AMP_GAIN | VERB_DATA(MUTE_SET_RAMP | MUTE_SET_LAMP | MUTE_SET_IN_AMP | MUTE_SET_INDEX(0)), NULL);
	if (ret)
		return ret;
	ret = widget_verb(hda, codec, mixer,
	                  VERB_CMD_SET_AMP_GAIN | VERB_DATA(MUTE_SET_RAMP | MUTE_SET_LAMP | MUTE_SET_IN_AMP | MUTE_SET_INDEX(1)), NULL);
	if (ret)
		return ret;
	return 0;
}

static int setup_codec(struct hda *hda, struct hda_codec *codec)
{
	if (codec->vendor == 0x1AF4
	 && codec->device == 0x0012)
		return setup_dev_qemu_output(hda, codec);
	if (codec->vendor == 0x1AF4
	 && codec->device == 0x0022)
		return setup_dev_qemu_duplex(hda, codec);
	if (codec->vendor == 0x1AF4
	 && codec->device == 0x0032)
		return setup_dev_qemu_micro(hda, codec);
	if (codec->vendor == 0x10EC
	 && codec->device == 0x0269)
		return setup_dev_alc269(hda, codec);
	TRACE("hda: unknown codec %04x:%04x", codec->vendor, codec->device);
	return -EINVAL;
}

static int setup_codecs(struct hda *hda)
{
	for (size_t i = 0; i < hda->codecs_count; ++i)
		setup_codec(hda, &hda->codecs[i]);
	return 0;
}

static int sysfs_open(struct file *file, struct node *node)
{
	file->userdata = node->userdata;
	return 0;
}

static ssize_t sysfs_read(struct file *file, struct uio *uio)
{
	struct hda *hda = file->userdata;
	size_t count = uio->count;
	off_t off = uio->off;
	uprintf(uio, "cap: 0x%04" PRIx16 ", version: %" PRIu8 ".%" PRIu8 "\n",
	        hda_r16(hda, REG_GCAP), hda_r8(hda, REG_VMAJ),
	        hda_r8(hda, REG_VMIN));
	print_codec(uio, &hda->codecs[0], 1);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op sysfs_fop =
{
	.open = sysfs_open,
	.read = sysfs_read,
};

static int register_sysfs(struct hda *hda)
{
	int ret = sysfs_mknode("hda", 0, 0, 0444, &sysfs_fop, &hda->sysfs_node);
	if (ret)
		return ret;
	hda->sysfs_node->userdata = hda;
	return 0;
}

void hda_free(struct hda *hda)
{
	if (!hda)
		return;
	dma_buf_free(hda->dmap);
	dma_buf_free(hda->corb);
	dma_buf_free(hda->rirb);
	snd_free(hda->out);
	snd_free(hda->in);
	dma_buf_free(hda->out_desc);
	dma_buf_free(hda->in_desc);
	pci_unmap(hda->device, hda->pci_map);
	/* XXX cdev destroy */
	free(hda);
}

int init_pci(struct pci_device *device, void *userdata)
{
	struct hda *hda = NULL;
	int ret;

	(void)userdata;
	if (device->header.vendor == 0x8086
	 && device->header.device == 0x1C20)
	{
		uint32_t devc = pci_dev_read(device, 0x78);
		devc &= ~0x800;
		pci_dev_write(device, 0x78, devc);
	}
	hda = malloc(sizeof(*hda), M_ZERO);
	if (!hda)
	{
		TRACE("hda: failed to allocate hda");
		ret = -ENOMEM;
		goto err;
	}
	hda->device = device;
	ret = snd_alloc(&hda->out);
	if (ret)
	{
		TRACE("hda: failed to create out snd");
		goto err;
	}
	ret = pci_map_bar(device, 0, PAGE_SIZE, 0, &hda->pci_map);
	if (ret)
	{
		TRACE("hda: failed to init bar0");
		goto err;
	}
	hda_w8(hda, REG_CORBCTL, 0);
	while (hda_r8(hda, REG_CORBCTL) & CORB_CTL_DMA) /* XXX timeout */
		;
	hda_w8(hda, REG_RIRBCTL, 0);
	while (hda_r8(hda, REG_RIRBCTL) & RIRB_CTL_DMA) /* XXX timeout */
		;
	hda_w32(hda, REG_GCTL, 0);
	while (hda_r32(hda, REG_GCTL) & GCTL_CRST) /* XXX timeout */
		;
	hda_w32(hda, REG_GCTL, GCTL_CRST);
	while (!(hda_r32(hda, REG_GCTL) & GCTL_CRST)) /* XXX timeout */
		;
	/* The software must wait at least 521 us (25 frames)
	 * after reading CRST as a 1 before assuming that
	 * codecs have all made status change requests
	 * and have been registered by the controller.
	 */
	static const struct timespec delay = {0, 521000};
	spinsleep(&delay);
#if 0
	TRACE("cap: 0x%04" PRIx16 ", version: %" PRIu8 ".%" PRIu8,
	      hda_r16(hda, REG_GCAP), hda_r8(hda, REG_VMAJ),
	      hda_r8(hda, REG_VMIN));
#endif
	hda_w32(hda, REG_SSYNC, 0xFF);
	ret = setup_desc(hda);
	if (ret)
		goto err;
	ret = setup_corb(hda);
	if (ret)
		goto err;
	ret = setup_rirb(hda);
	if (ret)
		goto err;
	ret = dma_buf_alloc(PAGE_SIZE, 0, &hda->dmap);
	if (ret)
	{
		TRACE("hda: dmap allocation failed");
		goto err;
	}
	memset(hda->dmap->data, 0, PAGE_SIZE);
	uintptr_t page_addr = pm_page_addr(hda->dmap->pages);
	hda_w32(hda, REG_DPLBASE, page_addr | 1);
	hda_w32(hda, REG_DPUBASE, sizeof(page_addr) > 4 ? (page_addr >> 32) : 0);
	ret = setup_stream(hda);
	if (ret)
		goto err;
	ret = parse_codecs(hda);
	if (ret)
		goto err;
#if 0
	print_codec(NULL, &hda->codecs[0], 0);
#endif
	ret = setup_codecs(hda);
	if (ret)
		goto err;
	ret = pci_register_irq(device, int_handler, hda, &hda->irq_handle);
	if (ret)
	{
		TRACE("hda: failed to enable irq");
		goto err;
	}
	hda_w32(hda, REG_INTCTL, INTCTL_GIE | INTCTL_CIE | INTCTL_SIE(4));
	hda_w16(hda, REG_SDCTL(4), SDCTL_RUN | SDCTL_IOCE | SDCTL_FEIE | SDCTL_DEIE);
	while (!(hda_r16(hda, REG_SDSTS(4)) & SDSTS_FIFORDY)) /* XXX timeout */
		;
	hda_w32(hda, REG_SSYNC, 0);
	register_sysfs(hda);
	return 0;

err:
	hda_free(hda);
	return ret;
}

static int init(void)
{
	pci_probe(0x8086, 0x2668, init_pci, NULL);
	pci_probe(0x8086, 0x1C20, init_pci, NULL);
	return 0;
}

static void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "hda",
	.init = init,
	.fini = fini,
};
