#ifndef ARM_CSR_H
#define ARM_CSR_H

#define PSR_M(n) (n)
#define PSR_M_USR PSR_M(0x10)
#define PSR_M_FIQ PSR_M(0x11)
#define PSR_M_IRQ PSR_M(0x12)
#define PSR_M_SVC PSR_M(0x13)
#define PSR_M_MON PSR_M(0x16)
#define PSR_M_ABT PSR_M(0x17)
#define PSR_M_UND PSR_M(0x1B)
#define PSR_T     (1 << 5)
#define PSR_F     (1 << 6)
#define PSR_I     (1 << 7)
#define PSR_A     (1 << 8)
#define PSR_E     (1 << 9)
#define PSR_J     (1 << 24)
#define PSR_Q     (1 << 27)
#define PSR_V     (1 << 28)
#define PSR_C     (1 << 29)
#define PSR_Z     (1 << 30)
#define PSR_N     (1 << 31)

#endif
