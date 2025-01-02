#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#define MEM_ARM9_REG_DISPCNT         0x000
#define MEM_ARM9_REG_DISPSTAT        0x004
#define MEM_ARM9_REG_VCOUNT          0x006

#define MEM_ARM9_REG_BG0CNT          0x008
#define MEM_ARM9_REG_BG1CNT          0x00A
#define MEM_ARM9_REG_BG2CNT          0x00C
#define MEM_ARM9_REG_BG3CNT          0x00E
#define MEM_ARM9_REG_BG0HOFS         0x010
#define MEM_ARM9_REG_BG0VOFS         0x012
#define MEM_ARM9_REG_BG1HOFS         0x014
#define MEM_ARM9_REG_BG1VOFS         0x016
#define MEM_ARM9_REG_BG2HOFS         0x018
#define MEM_ARM9_REG_BG2VOFS         0x01A
#define MEM_ARM9_REG_BG3HOFS         0x01C
#define MEM_ARM9_REG_BG3VOFS         0x01E
#define MEM_ARM9_REG_BG2PA           0x020
#define MEM_ARM9_REG_BG2PB           0x022
#define MEM_ARM9_REG_BG2PC           0x024
#define MEM_ARM9_REG_BG2PD           0x026
#define MEM_ARM9_REG_BG2X            0x028
#define MEM_ARM9_REG_BG2Y            0x02C
#define MEM_ARM9_REG_BG3PA           0x030
#define MEM_ARM9_REG_BG3PB           0x032
#define MEM_ARM9_REG_BG3PC           0x034
#define MEM_ARM9_REG_BG3PD           0x036
#define MEM_ARM9_REG_BG3X            0x038
#define MEM_ARM9_REG_BG3Y            0x03C
#define MEM_ARM9_REG_WIN0H           0x040
#define MEM_ARM9_REG_WIN1H           0x042
#define MEM_ARM9_REG_WIN0V           0x044
#define MEM_ARM9_REG_WIN1V           0x046
#define MEM_ARM9_REG_WININ           0x048
#define MEM_ARM9_REG_WINOUT          0x04A
#define MEM_ARM9_REG_MOSAIC          0x04C
#define MEM_ARM9_REG_BLDCNT          0x050
#define MEM_ARM9_REG_BLDALPHA        0x052
#define MEM_ARM9_REG_BLDY            0x054

#define MEM_ARM9_REG_DISP3DCNT       0x060
#define MEM_ARM9_REG_DISPCAPCNT      0x064
#define MEM_ARM9_REG_DISPMFIFO       0x068
#define MEM_ARM9_REG_MASTER_BRIGHT   0x06C

#define MEM_ARM9_REG_DMA0SAD         0x0B0
#define MEM_ARM9_REG_DMA0DAD         0x0B4
#define MEM_ARM9_REG_DMA0CNT_L       0x0B8
#define MEM_ARM9_REG_DMA0CNT_H       0x0BA
#define MEM_ARM9_REG_DMA1SAD         0x0BC
#define MEM_ARM9_REG_DMA1DAD         0x0C0
#define MEM_ARM9_REG_DMA1CNT_L       0x0C4
#define MEM_ARM9_REG_DMA1CNT_H       0x0C6
#define MEM_ARM9_REG_DMA2SAD         0x0C8
#define MEM_ARM9_REG_DMA2DAD         0x0CC
#define MEM_ARM9_REG_DMA2CNT_L       0x0D0
#define MEM_ARM9_REG_DMA2CNT_H       0x0D2
#define MEM_ARM9_REG_DMA3SAD         0x0D4
#define MEM_ARM9_REG_DMA3DAD         0x0D8
#define MEM_ARM9_REG_DMA3CNT_L       0x0DC
#define MEM_ARM9_REG_DMA3CNT_H       0x0DE
#define MEM_ARM9_REG_DMA0FILL        0x0E0
#define MEM_ARM9_REG_DMA1FILL        0x0E4
#define MEM_ARM9_REG_DMA2FILL        0x0E8
#define MEM_ARM9_REG_DMA3FILL        0x0EC

#define MEM_ARM9_REG_TM0CNT_L        0x100
#define MEM_ARM9_REG_TM0CNT_H        0x102
#define MEM_ARM9_REG_TM1CNT_L        0x104
#define MEM_ARM9_REG_TM1CNT_H        0x106
#define MEM_ARM9_REG_TM2CNT_L        0x108
#define MEM_ARM9_REG_TM2CNT_H        0x10A
#define MEM_ARM9_REG_TM3CNT_L        0x10C
#define MEM_ARM9_REG_TM3CNT_H        0x10E

#define MEM_ARM9_REG_KEYINPUT        0x130
#define MEM_ARM9_REG_KEYCNT          0x132

#define MEM_ARM9_REG_IPCSYNC         0x180
#define MEM_ARM9_REG_IPCFIFOCNT      0x184
#define MEM_ARM9_REG_IPCFIFOSEND     0x188

#define MEM_ARM9_REG_AUXSPICNT       0x1A0
#define MEM_ARM9_REG_AUXSPIDATA      0x1A2
#define MEM_ARM9_REG_ROMCTRL         0x1A4
#define MEM_ARM9_REG_ROMCMD          0x1A8
#define MEM_ARM9_REG_ROMSEED0_L      0x1B0
#define MEM_ARM9_REG_ROMSEED1_L      0x1B4
#define MEM_ARM9_REG_ROMSEED0_H      0x1B8
#define MEM_ARM9_REG_ROMSEED1_H      0x1BA

#define MEM_ARM9_REG_EXMEMCNT        0x204
#define MEM_ARM9_REG_IME             0x208
#define MEM_ARM9_REG_IE              0x210
#define MEM_ARM9_REG_IF              0x214

#define MEM_ARM9_REG_VRAMCNT_A       0x240
#define MEM_ARM9_REG_VRAMCNT_B       0x241
#define MEM_ARM9_REG_VRAMCNT_C       0x242
#define MEM_ARM9_REG_VRAMCNT_D       0x243
#define MEM_ARM9_REG_VRAMCNT_E       0x244
#define MEM_ARM9_REG_VRAMCNT_F       0x245
#define MEM_ARM9_REG_VRAMCNT_G       0x246
#define MEM_ARM9_REG_WRAMCNT         0x247
#define MEM_ARM9_REG_VRAMCNT_H       0x248
#define MEM_ARM9_REG_VRAMCNT_I       0x249

#define MEM_ARM9_REG_DIVCNT          0x280
#define MEM_ARM9_REG_DIV_NUMER       0x290
#define MEM_ARM9_REG_DIV_DENOM       0x298
#define MEM_ARM9_REG_DIV_RESULT      0x2A0
#define MEM_ARM9_REG_DIVREM_RESULT   0x2A8
#define MEM_ARM9_REG_SQRTCNT         0x2B0
#define MEM_ARM9_REG_SQRT_RESULT     0x2B4
#define MEM_ARM9_REG_SQRT_PARAM      0x2B8

#define MEM_ARM9_REG_POSTFLG         0x300
#define MEM_ARM9_REG_POWCNT1         0x304

#define MEM_ARM9_REG_RDLINES_COUNT   0x320
#define MEM_ARM9_REG_EDGE_COLOR      0x330
#define MEM_ARM9_REG_ALPHA_TEST_REF  0x340
#define MEM_ARM9_REG_CLEAR_COLOR     0x350
#define MEM_ARM9_REG_CLEAR_DEPTH     0x354
#define MEM_ARM9_REG_CLRIMAGE_OFFSET 0x356
#define MEM_ARM9_REG_FOG_COLOR       0x358
#define MEM_ARM9_REG_FOG_OFFSET      0x35C
#define MEM_ARM9_REG_FOG_TABLE       0x360
#define MEM_ARM9_REG_TOON_TABLE      0x380

#define MEM_ARM9_REG_GXFIFO          0x400

#define MEM_ARM9_REG_MTX_MODE        0x440
#define MEM_ARM9_REG_MTX_PUSH        0x444
#define MEM_ARM9_REG_MTX_POP         0x448
#define MEM_ARM9_REG_MTX_STORE       0x44C
#define MEM_ARM9_REG_MTX_RESTORE     0x450
#define MEM_ARM9_REG_MTX_IDENTITY    0x454
#define MEM_ARM9_REG_MTX_LOAD_4X4    0x458
#define MEM_ARM9_REG_MTX_LOAD_4X3    0x45C
#define MEM_ARM9_REG_MTX_MULT_4X4    0x460
#define MEM_ARM9_REG_MTX_MULT_4X3    0x464
#define MEM_ARM9_REG_MTX_MULT_3X3    0x468
#define MEM_ARM9_REG_MTX_SCALE       0x46C
#define MEM_ARM9_REG_MTX_TRANS       0x470
#define MEM_ARM9_REG_COLOR           0x480
#define MEM_ARM9_REG_NORMAL          0x484
#define MEM_ARM9_REG_TEXCOORD        0x488
#define MEM_ARM9_REG_VTX_16          0x48C
#define MEM_ARM9_REG_VTX_10          0x490
#define MEM_ARM9_REG_VTX_XY          0x494
#define MEM_ARM9_REG_VTX_XZ          0x498
#define MEM_ARM9_REG_VTX_YZ          0x49C
#define MEM_ARM9_REG_VTX_DIFF        0x4A0
#define MEM_ARM9_REG_POLYGON_ATTR    0x4A4
#define MEM_ARM9_REG_TEXIMAGE_PARAM  0x4A8
#define MEM_ARM9_REG_PLTT_BASE       0x4AC
#define MEM_ARM9_REG_DIF_AMB         0x4C0
#define MEM_ARM9_REG_SPE_EMI         0x4C4
#define MEM_ARM9_REG_LIGHT_VECTOR    0x4C8
#define MEM_ARM9_REG_LIGHT_COLOR     0x4CC
#define MEM_ARM9_REG_SHININESS       0x4D0
#define MEM_ARM9_REG_BEGIN_VTXS      0x500
#define MEM_ARM9_REG_END_VTXS        0x504
#define MEM_ARM9_REG_SWAP_BUFFERS    0x540
#define MEM_ARM9_REG_VIEWPORT        0x580
#define MEM_ARM9_REG_BOX_TEST        0x5C0
#define MEM_ARM9_REG_POS_TEST        0x5C4
#define MEM_ARM9_REG_VEC_TEST        0x5C8

#define MEM_ARM9_REG_GXSTAT          0x600
#define MEM_ARM9_REG_RAM_COUNT       0x604
#define MEM_ARM9_REG_DISP_1DOT_DEPTH 0x610
#define MEM_ARM9_REG_POS_RESULT      0x620
#define MEM_ARM9_REG_VEC_RESULT      0x630
#define MEM_ARM9_REG_CLIPMTX_RESULT  0x640
#define MEM_ARM9_REG_VECMTX_RESULT   0x680

#define MEM_ARM9_REG_IPCFIFORECV     0x100000
#define MEM_ARM9_REG_ROMDATA         0x100010




#define MEM_ARM7_REG_DISPSTAT        0x004
#define MEM_ARM7_REG_VCOUNT          0x006

#define MEM_ARM7_REG_DMA0SAD         0x0B0
#define MEM_ARM7_REG_DMA0DAD         0x0B4
#define MEM_ARM7_REG_DMA0CNT_L       0x0B8
#define MEM_ARM7_REG_DMA0CNT_H       0x0BA
#define MEM_ARM7_REG_DMA1SAD         0x0BC
#define MEM_ARM7_REG_DMA1DAD         0x0C0
#define MEM_ARM7_REG_DMA1CNT_L       0x0C4
#define MEM_ARM7_REG_DMA1CNT_H       0x0C6
#define MEM_ARM7_REG_DMA2SAD         0x0C8
#define MEM_ARM7_REG_DMA2DAD         0x0CC
#define MEM_ARM7_REG_DMA2CNT_L       0x0D0
#define MEM_ARM7_REG_DMA2CNT_H       0x0D2
#define MEM_ARM7_REG_DMA3SAD         0x0D4
#define MEM_ARM7_REG_DMA3DAD         0x0D8
#define MEM_ARM7_REG_DMA3CNT_L       0x0DC
#define MEM_ARM7_REG_DMA3CNT_H       0x0DE

#define MEM_ARM7_REG_TM0CNT_L        0x100
#define MEM_ARM7_REG_TM0CNT_H        0x102
#define MEM_ARM7_REG_TM1CNT_L        0x104
#define MEM_ARM7_REG_TM1CNT_H        0x106
#define MEM_ARM7_REG_TM2CNT_L        0x108
#define MEM_ARM7_REG_TM2CNT_H        0x10A
#define MEM_ARM7_REG_TM3CNT_L        0x10C
#define MEM_ARM7_REG_TM3CNT_H        0x10E

#define MEM_ARM7_REG_SIODATA32       0x120
#define MEM_ARM7_REG_SIOCNT          0x128

#define MEM_ARM7_REG_KEYINPUT        0x130
#define MEM_ARM7_REG_KEYCNT          0x132
#define MEM_ARM7_REG_RCNT            0x134
#define MEM_ARM7_REG_EXTKEYIN        0x136
#define MEM_ARM7_REG_RTC             0x138

#define MEM_ARM7_REG_IPCSYNC         0x180
#define MEM_ARM7_REG_IPCFIFOCNT      0x184
#define MEM_ARM7_REG_IPCFIFOSEND     0x188

#define MEM_ARM7_REG_AUXSPICNT       0x1A0
#define MEM_ARM7_REG_AUXSPIDATA      0x1A2
#define MEM_ARM7_REG_ROMCTRL         0x1A4
#define MEM_ARM7_REG_ROMCMD          0x1A8
#define MEM_ARM7_REG_ROMSEED0_L      0x1B0
#define MEM_ARM7_REG_ROMSEED1_L      0x1B4
#define MEM_ARM7_REG_ROMSEED0_H      0x1B8
#define MEM_ARM7_REG_ROMSEED1_H      0x1BA
#define MEM_ARM7_REG_SPICNT          0x1C0
#define MEM_ARM7_REG_SPIDATA         0x1C2

#define MEM_ARM7_REG_EXMEMSTAT       0x204
#define MEM_ARM7_REG_WIFIWAITCNT     0x206
#define MEM_ARM7_REG_IME             0x208
#define MEM_ARM7_REG_IE              0x210
#define MEM_ARM7_REG_IF              0x214

#define MEM_ARM7_REG_VRAMSTAT        0x240
#define MEM_ARM7_REG_WRAMSTAT        0x241

#define MEM_ARM7_REG_POSTFLG         0x300
#define MEM_ARM7_REG_HALTCNT         0x301
#define MEM_ARM7_REG_POWCNT2         0x304
#define MEM_ARM7_REG_BIOSPROT        0x308

#define MEM_ARM7_REG_SOUND0CNT       0x400
#define MEM_ARM7_REG_SOUND0SAD       0x404
#define MEM_ARM7_REG_SOUND0TMR       0x408
#define MEM_ARM7_REG_SOUND0PNT       0x40A
#define MEM_ARM7_REG_SOUND0LEN       0x40C
#define MEM_ARM7_REG_SOUNDXCNT(x)   (MEM_ARM7_REG_SOUND0CNT + (x) * 0x10)
#define MEM_ARM7_REG_SOUNDXSAD(x)   (MEM_ARM7_REG_SOUND0SAD + (x) * 0x10)
#define MEM_ARM7_REG_SOUNDXTMR(x)   (MEM_ARM7_REG_SOUND0TMR + (x) * 0x10)
#define MEM_ARM7_REG_SOUNDXPNT(x)   (MEM_ARM7_REG_SOUND0PNT + (x) * 0x10)
#define MEM_ARM7_REG_SOUNDXLEN(x)   (MEM_ARM7_REG_SOUND0LEN + (x) * 0x10)

#define MEM_ARM7_REG_SOUNDCNT        0x500
#define MEM_ARM7_REG_SOUNDBIAS       0x504
#define MEM_ARM7_REG_SNDCAP0CNT      0x508
#define MEM_ARM7_REG_SNDCAP1CNT      0x509
#define MEM_ARM7_REG_SNDCAP0DAD      0x510
#define MEM_ARM7_REG_SNDCAP0LEN      0x514
#define MEM_ARM7_REG_SNDCAP1DAD      0x518
#define MEM_ARM7_REG_SNDCAP1LEN      0x51C

#define MEM_ARM7_REG_IPCFIFORECV     0x100000
#define MEM_ARM7_REG_ROMDATA         0x100010

#define MEM_ARM7_REG_W_ID            0x808000
#define MEM_ARM7_REG_W_MODE_RST      0x808004
#define MEM_ARM7_REG_W_MODE_WEP      0x808006
#define MEM_ARM7_REG_W_TXSTATCNT     0x808008
#define MEM_ARM7_REG_W_X_00A         0x80800A
#define MEM_ARM7_REG_W_IF            0x808010
#define MEM_ARM7_REG_W_IE            0x808014
#define MEM_ARM7_REG_W_MACADDR_0     0x808018
#define MEM_ARM7_REG_W_MACADDR_1     0x80801A
#define MEM_ARM7_REG_W_MACADDR_2     0x80801C
#define MEM_ARM7_REG_W_BSSID_0       0x808020
#define MEM_ARM7_REG_W_BSSID_1       0x808022
#define MEM_ARM7_REG_W_BSSID_2       0x808024
#define MEM_ARM7_REG_W_AID_LOW       0x808028
#define MEM_ARM7_REG_W_AID_FULL      0x80802A
#define MEM_ARM7_REG_W_TX_RETRYLIMIT 0x80802C
#define MEM_ARM7_REG_W_INTERNAL_2E   0x80802E
#define MEM_ARM7_REG_W_RXCNT         0x808030
#define MEM_ARM7_REG_W_WEP_CNT       0x808032
#define MEM_ARM7_REG_W_INTERNAL_34   0x808034
#define MEM_ARM7_REG_W_POWER_US      0x808036
#define MEM_ARM7_REG_W_POWER_TX      0x808038
#define MEM_ARM7_REG_W_POWERSTATE    0x80803C
#define MEM_ARM7_REG_W_POWERFORCE    0x808040
#define MEM_ARM7_REG_W_RANDOM        0x808044
#define MEM_ARM7_REG_W_POWER_UNK     0x808048
#define MEM_ARM7_REG_W_RXBUF_BEGIN   0x808050
#define MEM_ARM7_REG_W_RXBUF_END     0x808052
#define MEM_ARM7_REG_W_RXBUF_WRCSR   0x808054
#define MEM_ARM7_REG_W_RXBUF_WR_ADDR 0x808056
#define MEM_ARM7_REG_W_RXBUF_RD_ADDR 0x808058
#define MEM_ARM7_REG_W_RXBUF_READCSR 0x80805A
#define MEM_ARM7_REG_W_RXBUF_COUNT   0x80805C
#define MEM_ARM7_REG_W_RXBUF_RD_DATA 0x808060
#define MEM_ARM7_REG_W_RXBUF_GAP     0x808062
#define MEM_ARM7_REG_W_RXBUF_GAPDISP 0x808064
#define MEM_ARM7_REG_W_TXBUF_WR_ADDR 0x808068
#define MEM_ARM7_REG_W_TXBUF_COUNT   0x80806C
#define MEM_ARM7_REG_W_TXBUF_WR_DATA 0x808070
#define MEM_ARM7_REG_W_TXBUF_GAP     0x808074
#define MEM_ARM7_REG_W_TXBUF_GAPDISP 0x808076
#define MEM_ARM7_REG_W_INTERNAL_78   0x808078
#define MEM_ARM7_REG_W_TXBUF_BEACON  0x808080
#define MEM_ARM7_REG_W_TXBUF_TIM     0x808084
#define MEM_ARM7_REG_W_LISTENCOUNT   0x808088
#define MEM_ARM7_REG_W_BEACONINT     0x80808C
#define MEM_ARM7_REG_W_LISTENINT     0x80808E
#define MEM_ARM7_REG_W_TXBUF_CMD     0x808090
#define MEM_ARM7_REG_W_TXBUF_REPLY1  0x808094
#define MEM_ARM7_REG_W_TXBUF_REPLY2  0x808098
#define MEM_ARM7_REG_W_INTERNAL_9C   0x80809C
#define MEM_ARM7_REG_W_TXBUF_LOC1    0x8080A0
#define MEM_ARM7_REG_W_TXBUF_LOC2    0x8080A4
#define MEM_ARM7_REG_W_TXBUF_LOC3    0x8080A8
#define MEM_ARM7_REG_W_TXREQ_RESET   0x8080AC
#define MEM_ARM7_REG_W_TXREQ_SET     0x8080AE
#define MEM_ARM7_REG_W_TXREQ_READ    0x8080B0
#define MEM_ARM7_REG_W_TXBUF_RESET   0x8080B4
#define MEM_ARM7_REG_W_TXBUSY        0x8080B6
#define MEM_ARM7_REG_W_TXSTAT        0x8080B8
#define MEM_ARM7_REG_W_INTERNAL      0x8080BA
#define MEM_ARM7_REG_W_PREAMBLE      0x8080BC
#define MEM_ARM7_REG_W_CMD_TOTALTIME 0x8080C0
#define MEM_ARM7_REG_W_CMD_REPLYTIME 0x8080C4
#define MEM_ARM7_REG_W_INTERNAL_C8   0x8080C8
#define MEM_ARM7_REG_W_RXFILTER      0x8080D0
#define MEM_ARM7_REG_W_CONFIG_0D4    0x8080D4
#define MEM_ARM7_REG_W_CONFIG_0D8    0x8080D8
#define MEM_ARM7_REG_W_RX_LEN_CROP   0x8080DA
#define MEM_ARM7_REG_W_RXFILTER2     0x8080E0
#define MEM_ARM7_REG_W_US_COUNTCNT   0x8080E8
#define MEM_ARM7_REG_W_US_COMPARECNT 0x8080EA
#define MEM_ARM7_REG_W_CONFIG_0EC    0x8080EC
#define MEM_ARM7_REG_W_CMD_COUNTCNT  0x8080EE
#define MEM_ARM7_REG_W_US_COMPARE0   0x8080F0
#define MEM_ARM7_REG_W_US_COMPARE1   0x8080F2
#define MEM_ARM7_REG_W_US_COMPARE2   0x8080F4
#define MEM_ARM7_REG_W_US_COMPARE3   0x8080F6
#define MEM_ARM7_REG_W_US_COUNT0     0x8080F8
#define MEM_ARM7_REG_W_US_COUNT1     0x8080FA
#define MEM_ARM7_REG_W_US_COUNT2     0x8080FC
#define MEM_ARM7_REG_W_US_COUNT3     0x8080FE
#define MEM_ARM7_REG_W_INTERNAL_100  0x808100
#define MEM_ARM7_REG_W_INTERNAL_102  0x808102
#define MEM_ARM7_REG_W_INTERNAL_104  0x808104
#define MEM_ARM7_REG_W_INTERNAL_106  0x808106
#define MEM_ARM7_REG_W_CONTENTFREE   0x80810C
#define MEM_ARM7_REG_W_PRE_BEACON    0x808110
#define MEM_ARM7_REG_W_CMD_COUNT     0x808118
#define MEM_ARM7_REG_W_BEACON_COUNT  0x80811C
#define MEM_ARM7_REG_W_CONFIG_120    0x808120
#define MEM_ARM7_REG_W_CONFIG_122    0x808122
#define MEM_ARM7_REG_W_CONFIG_124    0x808124
#define MEM_ARM7_REG_W_INTERNAL_126  0x808126
#define MEM_ARM7_REG_W_CONFIG_128    0x808128
#define MEM_ARM7_REG_W_CONFIG_130    0x808130
#define MEM_ARM7_REG_W_CONFIG_132    0x808132
#define MEM_ARM7_REG_W_POST_BEACON   0x808134
#define MEM_ARM7_REG_W_CONFIG_140    0x808140
#define MEM_ARM7_REG_W_CONFIG_142    0x808142
#define MEM_ARM7_REG_W_CONFIG_144    0x808144
#define MEM_ARM7_REG_W_CONFIG_146    0x808146
#define MEM_ARM7_REG_W_CONFIG_148    0x808148
#define MEM_ARM7_REG_W_CONFIG_14A    0x80814A
#define MEM_ARM7_REG_W_CONFIG_14C    0x80814C
#define MEM_ARM7_REG_W_CONFIG_150    0x808150
#define MEM_ARM7_REG_W_CONFIG_154    0x808154
#define MEM_ARM7_REG_W_BB_CNT        0x808158
#define MEM_ARM7_REG_W_BB_WRITE      0x80815A
#define MEM_ARM7_REG_W_BB_READ       0x80815C
#define MEM_ARM7_REG_W_BB_BUSY       0x80815E
#define MEM_ARM7_REG_W_BB_MODE       0x808160
#define MEM_ARM7_REG_W_BB_POWER      0x808168
#define MEM_ARM7_REG_W_INTERNAL_16A  0x80816A
#define MEM_ARM7_REG_W_INTERNAL_170  0x808170
#define MEM_ARM7_REG_W_INTERNAL_172  0x808172
#define MEM_ARM7_REG_W_INTERNAL_174  0x808174
#define MEM_ARM7_REG_W_INTERNAL_176  0x808176
#define MEM_ARM7_REG_W_INTERNAL_178  0x808178
#define MEM_ARM7_REG_W_RF_DATA2      0x80817C
#define MEM_ARM7_REG_W_RF_DATA1      0x80817E
#define MEM_ARM7_REG_W_RF_BUSY       0x808180
#define MEM_ARM7_REG_W_RF_CNT        0x808184
#define MEM_ARM7_REG_W_INTERNAL_190  0x808190
#define MEM_ARM7_REG_W_TX_HDR_CNT    0x808194
#define MEM_ARM7_REG_W_INTERNAL_198  0x808198
#define MEM_ARM7_REG_W_RF_PINS       0x80819C
#define MEM_ARM7_REG_W_X_1A0         0x8081A0
#define MEM_ARM7_REG_W_X_1A2         0x8081A2
#define MEM_ARM7_REG_W_X_1A4         0x8081A4
#define MEM_ARM7_REG_W_RXSTAT_INC_IF 0x8081A8
#define MEM_ARM7_REG_W_RXSTAT_INC_IE 0x8081AA
#define MEM_ARM7_REG_W_RXSTAT_OVF_IF 0x8081AC
#define MEM_ARM7_REG_W_RXSTAT_OVF_IE 0x8081AE
#define MEM_ARM7_REG_W_RXSTAT_1B0    0x8081B0
#define MEM_ARM7_REG_W_RXSTAT_1B2    0x8081B2
#define MEM_ARM7_REG_W_RXSTAT_1B4    0x8081B4
#define MEM_ARM7_REG_W_RXSTAT_1B6    0x8081B6
#define MEM_ARM7_REG_W_RXSTAT_1B8    0x8081B8
#define MEM_ARM7_REG_W_RXSTAT_1BA    0x8081BA
#define MEM_ARM7_REG_W_RXSTAT_1BC    0x8081BC
#define MEM_ARM7_REG_W_RXSTAT_1BE    0x8081BE
#define MEM_ARM7_REG_W_TX_ERR_COUNT  0x8081C0
#define MEM_ARM7_REG_W_RX_COUNT      0x8081C4
#define MEM_ARM7_REG_W_CMD_STAT_1D0  0x8081D0
#define MEM_ARM7_REG_W_CMD_STAT_1D2  0x8081D2
#define MEM_ARM7_REG_W_CMD_STAT_1D4  0x8081D4
#define MEM_ARM7_REG_W_CMD_STAT_1D6  0x8081D6
#define MEM_ARM7_REG_W_CMD_STAT_1D8  0x8081D8
#define MEM_ARM7_REG_W_CMD_STAT_1DA  0x8081DA
#define MEM_ARM7_REG_W_CMD_STAT_1DC  0x8081DC
#define MEM_ARM7_REG_W_CMD_STAT_1DE  0x8081DE
#define MEM_ARM7_REG_W_INTERNAL_1F0  0x8081F0
#define MEM_ARM7_REG_W_INTERNAL_204  0x808204
#define MEM_ARM7_REG_W_INTERNAL_208  0x808208
#define MEM_ARM7_REG_W_INTERNAL_20C  0x80820C
#define MEM_ARM7_REG_W_TX_SEQNO      0x808210
#define MEM_ARM7_REG_W_RF_STATUS     0x808214
#define MEM_ARM7_REG_W_IF_SET        0x80821C
#define MEM_ARM7_REG_W_RAM_DISABLE   0x808220

enum mem_type
{
	MEM_DIRECT,
	MEM_DATA_SEQ,
	MEM_DATA_NSEQ,
	MEM_CODE_SEQ,
	MEM_CODE_NSEQ,
};

struct timer
{
	uint32_t v;
};

#define MEM_DMA_ACTIVE (1 << 0)
#define MEM_DMA_ENABLE (1 << 1)

struct dma
{
	uint8_t status;
	uint32_t src;
	uint32_t dst;
	uint32_t len;
	uint32_t cnt;
};

struct fifo
{
	uint8_t data[64];
	uint8_t len;
	uint8_t pos;
	uint8_t latch[4];
};

#define SPI_FIRMWARE_CMD_NONE 0x00
#define SPI_FIRMWARE_CMD_PP   0x02
#define SPI_FIRMWARE_CMD_READ 0x03
#define SPI_FIRMWARE_CMD_WRDI 0x04
#define SPI_FIRMWARE_CMD_RDSR 0x05
#define SPI_FIRMWARE_CMD_WREN 0x06
#define SPI_FIRMWARE_CMD_PW   0x0A
#define SPI_FIRMWARE_CMD_FAST 0x0B
#define SPI_FIRMWARE_CMD_RDP  0xAB
#define SPI_FIRMWARE_CMD_DP   0xB9
#define SPI_FIRMWARE_CMD_SE   0xD8
#define SPI_FIRMWARE_CMD_PE   0xDB
#define SPI_FIRMWARE_CMD_RDID 0x9F

struct spi_firmware
{
	uint8_t cmd;
	uint8_t posb;
	uint8_t write;
	uint8_t read_latch;
	uint32_t addr;
};

struct spi_powerman
{
	uint8_t has_cmd;
	uint8_t cmd;
	uint8_t read_latch;
	uint8_t regs[0x5];
};

struct spi_touchscreen
{
	uint16_t read_latch;
	uint8_t read_pos;
	uint8_t channel;
	uint8_t has_channel;
};

struct rtc
{
	int cmd_flip;
	uint8_t cmd;
	uint8_t inbuf;
	uint8_t inlen; /* in bits */
	uint8_t outbuf[8];
	uint8_t outlen; /* in bits */
	uint8_t outpos; /* in bits */
	uint8_t outbyte;
	uint8_t wpos;
	uint8_t sr1;
	uint8_t sr2;
	uint8_t fr;
	uint8_t car;
	uint8_t int1_steady_freq;
	uint8_t alarm1[3];
	uint8_t alarm2[3];
	int64_t offset;
	struct tm tm; /* for date / time set */
};

#define GX_CMD_MTX_MODE       0x10
#define GX_CMD_MTX_PUSH       0x11
#define GX_CMD_MTX_POP        0x12
#define GX_CMD_MTX_STORE      0x13
#define GX_CMD_MTX_RESTORE    0x14
#define GX_CMD_MTX_IDENTITY   0x15
#define GX_CMD_MTX_LOAD_4X4   0x16
#define GX_CMD_MTX_LOAD_4X3   0x17
#define GX_CMD_MTX_MULT_4X4   0x18
#define GX_CMD_MTX_MULT_4X3   0x19
#define GX_CMD_MTX_MULT_3X3   0x1A
#define GX_CMD_MTX_SCALE      0x1B
#define GX_CMD_MTX_TRANS      0x1C
#define GX_CMD_COLOR          0x20
#define GX_CMD_NORMAL         0x21
#define GX_CMD_TEXCOORD       0x22
#define GX_CMD_VTX_16         0x23
#define GX_CMD_VTX_10         0x24
#define GX_CMD_VTX_XY         0x25
#define GX_CMD_VTX_XZ         0x26
#define GX_CMD_VTX_YZ         0x27
#define GX_CMD_VTX_DIFF       0x28
#define GX_CMD_POLYGON_ATTR   0x29
#define GX_CMD_TEXIMAGE_PARAM 0x2A
#define GX_CMD_PLTT_BASE      0x2B
#define GX_CMD_DIF_AMB        0x30
#define GX_CMD_SPE_EMI        0x31
#define GX_CMD_LIGHT_VECTOR   0x32
#define GX_CMD_LIGHT_COLOR    0x33
#define GX_CMD_SHININESS      0x34
#define GX_CMD_BEGIN_VTXS     0x40
#define GX_CMD_END_VTXS       0x41
#define GX_CMD_SWAP_BUFFERS   0x50
#define GX_CMD_VIEWPORT       0x60
#define GX_CMD_BOX_TEST       0x70
#define GX_CMD_POS_TEST       0x71
#define GX_CMD_VEC_TEST       0x72

struct gx_cmd_def
{
	const char *name;
	uint8_t params;
};

struct gx_cmd
{
	const struct gx_cmd_def *def;
	uint32_t params[32];
	uint8_t params_nb;
};

struct wifi
{
	uint8_t bb_regs[0xFF];
	uint8_t regs[0x300];
	uint8_t xbuf[0x2000];
};

#define MEM_VRAM_A_BASE 0x00000
#define MEM_VRAM_A_MASK 0x1FFFF
#define MEM_VRAM_B_BASE 0x20000
#define MEM_VRAM_B_MASK 0x1FFFF
#define MEM_VRAM_C_BASE 0x40000
#define MEM_VRAM_C_MASK 0x1FFFF
#define MEM_VRAM_D_BASE 0x60000
#define MEM_VRAM_D_MASK 0x1FFFF
#define MEM_VRAM_E_BASE 0x80000
#define MEM_VRAM_E_MASK 0x0FFFF
#define MEM_VRAM_F_BASE 0x90000
#define MEM_VRAM_F_MASK 0x03FFF
#define MEM_VRAM_G_BASE 0x94000
#define MEM_VRAM_G_MASK 0x03FFF
#define MEM_VRAM_H_BASE 0x98000
#define MEM_VRAM_H_MASK 0x07FFF
#define MEM_VRAM_I_BASE 0xA0000
#define MEM_VRAM_I_MASK 0x03FFF

struct nds;
struct mbc;

struct mem
{
	struct nds *nds;
	struct mbc *mbc;
	struct timer arm7_timers[4];
	struct timer arm9_timers[4];
	struct dma arm7_dma[4];
	struct dma arm9_dma[4];
	struct fifo arm7_fifo;
	struct fifo arm9_fifo;
	struct spi_firmware spi_firmware;
	struct spi_powerman spi_powerman;
	struct spi_touchscreen spi_touchscreen;
	struct rtc rtc;
	struct wifi wifi;
	uint8_t arm7_bios[0x4000];
	uint8_t arm9_bios[0x1000];
	uint8_t firmware[0x40000];
	uint8_t arm7_regs[0x600];
	uint8_t arm9_regs[0x1070];
	uint8_t mram[0x400000];
	uint8_t wram[0x8000];
	uint8_t arm7_wram[0x10000];
	uint32_t arm7_wram_base;
	uint32_t arm7_wram_mask;
	uint32_t arm9_wram_base;
	uint32_t arm9_wram_mask;
	uint8_t dtcm[0x4000];
	uint8_t itcm[0x8000];
	uint8_t vram[0xA4000];
	uint8_t oam[0x800];
	uint8_t palette[0x800];
	int biosprot;
	uint32_t vram_bga_bases[32]; /* 0x4000 units */
	uint32_t vram_bgb_bases[8]; /* 0x4000 units */
	uint32_t vram_obja_bases[16]; /* 0x4000 units */
	uint32_t vram_objb_bases[8]; /* 0x4000 units */
	uint32_t vram_arm7_bases[2]; /* 0x20000 units */
	uint32_t vram_bgepa_bases[2]; /* 0x4000 units */
	uint32_t vram_bgepb_base; /* 0x8000 */
	uint32_t vram_objepa_base; /* 0x2000 */
	uint32_t vram_objepb_base; /* 0x2000 */
	uint32_t vram_trpi_bases[4]; /* 0x20000 units */
	uint32_t vram_texp_bases[8]; /* 0x4000 units, (only 6 effective) */
	uint8_t *sram; /* backup + firmware sram */
	size_t sram_size;
	uint8_t dscard_dma_count; /* XXX this should be removed */
	uint8_t gxfifo_dma_count; /* XXX this should be removed */
	uint32_t itcm_base;
	uint32_t itcm_mask;
	uint32_t dtcm_base;
	uint32_t dtcm_mask;
	struct gx_cmd gx_cmd[4];
	uint8_t gx_cmd_nb;
};

struct mem *mem_new(struct nds *nds, struct mbc *mbc);
void mem_del(struct mem *mem);

void mem_timers(struct mem *mem, uint32_t cycles);
void mem_dma(struct mem *mem, uint32_t cycles);
void mem_vblank(struct mem *mem);
void mem_hblank(struct mem *mem);
void mem_dscard(struct mem *mem);

void mem_arm9_irq(struct mem *mem, uint32_t f);
void mem_arm7_irq(struct mem *mem, uint32_t f);

uint8_t  mem_arm7_get8 (struct mem *mem, uint32_t addr, enum mem_type type);
uint16_t mem_arm7_get16(struct mem *mem, uint32_t addr, enum mem_type type);
uint32_t mem_arm7_get32(struct mem *mem, uint32_t addr, enum mem_type type);
void mem_arm7_set8 (struct mem *mem, uint32_t addr, uint8_t val, enum mem_type type);
void mem_arm7_set16(struct mem *mem, uint32_t addr, uint16_t val, enum mem_type type);
void mem_arm7_set32(struct mem *mem, uint32_t addr, uint32_t val, enum mem_type type);

uint8_t  mem_arm9_get8 (struct mem *mem, uint32_t addr, enum mem_type type);
uint16_t mem_arm9_get16(struct mem *mem, uint32_t addr, enum mem_type type);
uint32_t mem_arm9_get32(struct mem *mem, uint32_t addr, enum mem_type type);
void mem_arm9_set8 (struct mem *mem, uint32_t addr, uint8_t val, enum mem_type type);
void mem_arm9_set16(struct mem *mem, uint32_t addr, uint16_t val, enum mem_type type);
void mem_arm9_set32(struct mem *mem, uint32_t addr, uint32_t val, enum mem_type type);

void *get_arm9_vram_ptr(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_bga_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_bga_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_bga_get32(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_bgb_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_bgb_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_bgb_get32(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_obja_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_obja_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_obja_get32(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_objb_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_objb_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_objb_get32(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_bgepa_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_bgepa_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_bgepa_get32(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_bgepb_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_bgepb_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_bgepb_get32(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_objepa_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_objepa_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_objepa_get32(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_objepb_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_objepb_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_objepb_get32(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_trpi_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_trpi_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_trpi_get32(struct mem *mem, uint32_t addr);
uint8_t  mem_vram_texp_get8 (struct mem *mem, uint32_t addr);
uint16_t mem_vram_texp_get16(struct mem *mem, uint32_t addr);
uint32_t mem_vram_texp_get32(struct mem *mem, uint32_t addr);

static inline uint8_t mem_arm9_get_reg8(struct mem *mem, uint32_t reg)
{
	return mem->arm9_regs[reg];
}

static inline void mem_arm9_set_reg8(struct mem *mem, uint32_t reg, uint8_t val)
{
	mem->arm9_regs[reg] = val;
}

static inline uint16_t mem_arm9_get_reg16(struct mem *mem, uint32_t reg)
{
	return *(uint16_t*)&mem->arm9_regs[reg];
}

static inline void mem_arm9_set_reg16(struct mem *mem, uint32_t reg, uint16_t val)
{
	*(uint16_t*)&mem->arm9_regs[reg] = val;
}

static inline uint32_t mem_arm9_get_reg32(struct mem *mem, uint32_t reg)
{
	return *(uint32_t*)&mem->arm9_regs[reg];
}

static inline void mem_arm9_set_reg32(struct mem *mem, uint32_t reg, uint32_t val)
{
	*(uint32_t*)&mem->arm9_regs[reg] = val;
}

static inline uint64_t mem_arm9_get_reg64(struct mem *mem, uint32_t reg)
{
	return *(uint64_t*)&mem->arm9_regs[reg];
}

static inline void mem_arm9_set_reg64(struct mem *mem, uint32_t reg, uint64_t val)
{
	*(uint64_t*)&mem->arm9_regs[reg] = val;
}

static inline uint8_t mem_arm7_get_reg8(struct mem *mem, uint32_t reg)
{
	return mem->arm7_regs[reg];
}

static inline void mem_arm7_set_reg8(struct mem *mem, uint32_t reg, uint8_t val)
{
	mem->arm7_regs[reg] = val;
}

static inline uint16_t mem_arm7_get_reg16(struct mem *mem, uint32_t reg)
{
	return *(uint16_t*)&mem->arm7_regs[reg];
}

static inline void mem_arm7_set_reg16(struct mem *mem, uint32_t reg, uint16_t val)
{
	*(uint16_t*)&mem->arm7_regs[reg] = val;
}

static inline uint32_t mem_arm7_get_reg32(struct mem *mem, uint32_t reg)
{
	return *(uint32_t*)&mem->arm7_regs[reg];
}

static inline void mem_arm7_set_reg32(struct mem *mem, uint32_t reg, uint32_t val)
{
	*(uint32_t*)&mem->arm7_regs[reg] = val;
}

static inline void mem_set_vram16(struct mem *mem, uint32_t addr, uint16_t val)
{
	*(uint16_t*)&mem->vram[addr] = val;
}

static inline uint16_t mem_get_oam16(struct mem *mem, uint32_t addr)
{
	return *(uint16_t*)&mem->oam[addr];
}

static inline uint16_t mem_get_bg_palette(struct mem *mem, uint32_t addr)
{
	return *(uint16_t*)&mem->palette[addr];
}

static inline uint16_t mem_get_obj_palette(struct mem *mem, uint32_t addr)
{
	return *(uint16_t*)&mem->palette[0x200 + addr];
}

static inline uint8_t mem_get_wifi_reg8(struct mem *mem, uint32_t addr)
{
	return *(uint8_t*)&mem->wifi.regs[addr - MEM_ARM7_REG_W_ID];
}

static inline void mem_set_wifi_reg8(struct mem *mem, uint32_t addr, uint8_t v)
{
	*(uint8_t*)&mem->wifi.regs[addr - MEM_ARM7_REG_W_ID] = v;
}

static inline uint16_t mem_get_wifi_reg16(struct mem *mem, uint32_t addr)
{
	return *(uint16_t*)&mem->wifi.regs[addr - MEM_ARM7_REG_W_ID];
}

static inline void mem_set_wifi_reg16(struct mem *mem, uint32_t addr, uint16_t v)
{
	*(uint16_t*)&mem->wifi.regs[addr - MEM_ARM7_REG_W_ID] = v;
}

#endif
