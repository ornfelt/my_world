#ifndef RISCV_CSR_H
#define RISCV_CSR_H

#define CSR_FFLAGS 0x001
#define CSR_FRM    0x002
#define CSR_FCSR   0x003

#define CSR_SSTATUS    0x100 /* Supervisor Status */
#define CSR_SEDELEG    0x102
#define CSR_SIDELEG    0x103
#define CSR_SIE        0x104 /* Supervisor Interrupt Enable */
#define CSR_STVEC      0x105 /* Supervisor Trap Vector */
#define CSR_SCOUNTEREN 0x106 /* Supervisor Counter Enable */

#define CSR_SSCRATCH 0x140 /* Supervisor Scratch Register */
#define CSR_SEPC     0x141 /* Supervisor Exception Program Counter */
#define CSR_SCAUSE   0x142 /* Supervisor Cause */
#define CSR_STVAL    0x143 /* Supervisor Trap Value */
#define CSR_SIP      0x144 /* Supervisor Interrupt Pending */

#define CSR_STIMECMP  0x14D
#define CSR_STIMECMPH 0x15D

#define CSR_SATP 0x180 /* Supervisor Address Translation and Protection */

#define CSR_MSTATUS    0x300 /* Machine Status */
#define CSR_MISA       0x301 /* Machine ISA Control */
#define CSR_MEDELEG    0x302 /* Machine Exception Delegation */
#define CSR_MIDELEG    0x303 /* Machine Interrupt Delegation */
#define CSR_MIE        0x304 /* Machine Interrupt Enable */
#define CSR_MTVEC      0x305 /* Machine Trap Vector Control */
#define CSR_MCOUNTEREN 0x306 /* Machine Counter Enable */
#define CSR_MSTATUSH   0x307

#define CSR_MCOUNTINHIBIT 0x320 /* Machine Counter Inhibit */
#define CSR_MHPMEVENT3    0x323

#define CSR_MSCRATCH 0x340 /* Machine Scratch Register */
#define CSR_MEPC     0x341 /* Machine Exception Program Counter */
#define CSR_MCAUSE   0x342 /* Machine Cause */
#define CSR_MTVAL    0x343 /* Machine Trap Value */
#define CSR_MIP      0x344 /* Machine Interrupt Pending */
#define CSR_MTINST   0x34A
#define CSR_MTVAL2   0x34B

#define CSR_PMPCFG0  0x3A0 /* PMP Configuration Register 0 */
#define CSR_PMPADDR0 0x3B0 /* PMP Address 0 */

#define CSR_TSELECT 0x7A0
#define CSR_TDATA1  0x7A1
#define CSR_TDATA2  0x7A2
#define CSR_TDATA3  0x7A3

#define CSR_DCSR      0x7B0
#define CSR_DPC       0x7B1
#define CSR_DSCRATCH0 0x7B2
#define CSR_DSCRATCH1 0x7B3

#define CSR_MCYCLE       0xB00 /* Machine Cycle Counter */
#define CSR_MINSTRET     0xB02 /* Machine Instructions Retired Counter */
#define CSR_MHPMCOUNTER3 0xB03 /* Machine Hardware Performance Counter 3 */

#define CSR_MCYCLEH       0xB80
#define CSR_MINSTRETH     0xB82 /* Machine Instructions Retired Counter */
#define CSR_MHPMCOUNTER3H 0xB83

#define CSR_CYCLE       0xC00 /* Cycle counter for RDCYCLE Instruction */
#define CSR_TIME        0xC01 /* Timer for RDTIME Instruction */
#define CSR_INSTRET     0xC02 /* Instructions retired counter for RDINSTRET Instruction */
#define CSR_HPMCOUNTER3 0xC03 /* Cycle counter for RDCYCLE Instruction */

#define CSR_CYCLEH       0xC80 /* Cycle counter for RDCYCLE Instruction, high half */
#define CSR_TIMEH        0xC81 /* Timer for RDTIME Instruction, high half */
#define CSR_INSTRETH     0xC82 /* Instructions retired counter for RDINSTRET Instruction, high half */
#define CSR_HPMCOUNTERH3 0xC83 /* Cycle counter for RDCYCLE Instruction, high half */

#define CSR_MVENDORID 0xF11 /* Machine Vendor ID */
#define CSR_MARCHID   0xF12 /* Machine Architecture ID */
#define CSR_MIMPID    0xF13 /* Machine Implementation ID */
#define CSR_MHARTID   0xF14 /* Machine Hart ID */

#define CSR_MSTATUS_UIE    (1 << 0) /* U-mode Interrupt Enable */
#define CSR_MSTATUS_SIE    (1 << 1) /* S-mode Interrupt Enable */
#define CSR_MSTATUS_MIE    (1 << 3) /* M-mode Interrupt Enable */
#define CSR_MSTATUS_UPIE   (1 << 4) /* U-mode Previous Interrupt Enable */
#define CSR_MSTATUS_SPIE   (1 << 5) /* S-mode Previous Interrupt Enable */
#define CSR_MSTATUS_UBE    (1 << 6) /* U-mode Big Endian */
#define CSR_MSTATUS_MPIE   (1 << 7) /* M-mode Previous Interrupt Enable */
#define CSR_MSTATUS_SPP    (1 << 8) /* S-mode Previous Privilege */
#define CSR_MSTATUS_MPP(n) ((n) << 11) /* M-mode Previous Privilege */
#define CSR_MSTATUS_FS(n)  ((n) << 13) /* Floating point context Status */
#define CSR_MSTATUS_XS(n)  ((n) << 15) /* Custom (X) extension context Status */
#define CSR_MSTATUS_MRPV   (1 << 17) /* Modify PRiVilege */
#define CSR_MSTATUS_SUM    (1 << 18) /* permit Supervisor Memory Access */
#define CSR_MSTATUS_MXR    (1 << 19) /* Make eXecutable Readable */
#define CSR_MSTATUS_TVM    (1 << 20) /* Trap Virtual Memory */
#define CSR_MSTATUS_TW     (1 << 21) /* Timeout Wait */
#define CSR_MSTATUS_TSR    (1 << 22) /* Trap SRET */
#if __riscv_xlen == 64
#define CSR_MSTATUS_UXL(n) ((n) << 32) /* U-mode XLEN */
#define CSR_MSTATUS_SXL(n) ((n) << 34) /* S-mode XLEN */
#define CSR_MSTATUS_SBE    (1 << 36) /* S-mode Big Endian */
#define CSR_MSTATUS_MBE    (1 << 37) /* M-mode Big Endian */
#define CSR_MSTATUS_GVA    (1 << 38) /* Guest Virtual Address */
#define CSR_MSTATUS_MPV    (1 << 39) /* Machine Previous Virtualization mode */
#define CSR_MSTATUS_SD     (1 << 63) /* State Dirty */
#else
#define CSR_MSTATUS_SD     (1 << 31) /* State Dirty */
#endif

#define CSR_SSTATUS_UIE    (1 << 0) /* U-mode Interrupt Enable */
#define CSR_SSTATUS_SIE    (1 << 1) /* S-mode Interrupt Enable */
#define CSR_SSTATUS_UPIE   (1 << 4) /* U-mode Previous Interrupt Enable */
#define CSR_SSTATUS_SPIE   (1 << 5) /* S-mode Previous Interrupt Enable */
#define CSR_SSTATUS_UBE    (1 << 6) /* U-mode Big Endian */
#define CSR_SSTATUS_SPP    (1 << 8) /* S-mode Previous Privilege */
#define CSR_SSTATUS_FS(n)  ((n) << 13) /* Floating point context Status */
#define CSR_SSTATUS_XS(n)  ((n) << 15) /* Custom (X) extension context Status */
#define CSR_SSTATUS_SUM    (1 << 18) /* permit Supervisor Memory Access */
#define CSR_SSTATUS_MXR    (1 << 19) /* Make eXecutable Readable */
#if __riscv_xlen == 64
#define CSR_SSTATUS_UXL(n) ((n) << 32) /* U-mode XLEN */
#define CSR_SSTATUS_SD     (1 << 63) /* State Dirty */
#else
#define CSR_SSTATUS_SD     (1 << 31) /* State Dirty */
#endif

#define CSR_SIP_SSIP   (1 << 1) /* Supervisor Software Interrupt Pending */
#define CSR_SIP_STIP   (1 << 5) /* Supervisor Timer Interrupt Pending */
#define CSR_SIP_SEIP   (1 << 9) /* Supervisor External Interrupt Pending */
#define CSR_SIP_LCOFIP (1 << 13) /* Local Counter Overflow Interrupt pending */

#define CSR_SIE_SSIE (1 << 1) /* Supervisor Software Interrupt Enable */
#define CSR_SIE_STIE (1 << 5) /* Supervisor Timer Interrupt Enable */
#define CSR_SIE_SEIE (1 << 9) /* Supervisor External Interrupt Enable */

#endif
