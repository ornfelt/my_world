#ifndef AARCH64_CSR_H
#define AARCH64_CSR_H

#define PSR_M(n) (n)
#define PSR_M_EL0T PSR_M(0)
#define PSR_M_EL1T PSR_M(4)
#define PSR_M_EL1H PSR_M(5)
#define PSR_F      (1 << 6)
#define PSR_I      (1 << 7)
#define PSR_A      (1 << 8)
#define PSR_D      (1 << 9)
#define PSR_IL     (1 << 20)
#define PSR_SS     (1 << 21)
#define PSR_V      (1 << 28)
#define PSR_C      (1 << 29)
#define PSR_Z      (1 << 30)
#define PSR_N      (1 << 31)

#define FPCR_FIZ   (1 << 0)
#define FPCR_AH    (1 << 1)
#define FPCR_NEP   (1 << 2)
#define FPCR_IOE   (1 << 8)
#define FPCR_DZE   (1 << 9)
#define FPCR_OFE   (1 << 10)
#define FPCR_UFE   (1 << 11)
#define FPCR_IXE   (1 << 12)
#define FPCR_IDE   (1 << 15)
#define FPCR_FZ16  (1 << 19)
#define FPCR_RM(n) ((n) << 22)
#define FPCR_FZ    (1 << 24)
#define FPCR_DN    (1 << 25)
#define FPCR_AHP   (1 << 26)

#define FPSR_IOC (1 << 0)
#define FPSR_DZC (1 << 1)
#define FPSR_OFC (1 << 2)
#define FPSR_UFC (1 << 3)
#define FPSR_IXC (1 << 4)
#define FPSR_IDC (1 << 7)
#define FPSR_QC  (1 << 27)
#define FPSR_V   (1 << 28)
#define FPSR_C   (1 << 29)
#define FPSR_Z   (1 << 30)
#define FPSR_N   (1 << 31)

#endif