#ifndef RISCV_SBI_H
#define RISCV_SBI_H

#include <types.h>

#define SBI_EXT_TIMER   0x54494D45
#define SBI_EXT_IPI     0x735049
#define SBI_EXT_RFENCE  0x52464E43
#define SBI_EXT_HSM     0x48534D
#define SBI_EXT_RESET   0x53525354
#define SBI_EXT_CONSOLE 0x4442434E

/* base extension */
int sbi_get_spec_version(uintptr_t *spec_version);
int sbi_get_impl_id(uintptr_t *impl_id);
int sbi_get_impl_version(uintptr_t *impl_version);
int sbi_probe_extension(uintptr_t ext_id, uintptr_t *implemented);
int sbi_get_mvendorid(uintptr_t *mvendorid);
int sbi_get_marchid(uintptr_t *marchid);
int sbi_get_mimpid(uintptr_t *mimpid);

/* timer extension */
int sbi_set_timer(uint64_t stime_value);

/* ipi extension */
int sbi_send_ipi(uintptr_t hart_mask, uintptr_t hart_mask_base);

/* rfence extension */
int sbi_remote_fence_i(uintptr_t hart_mask,
                       uintptr_t hart_mask_base);
int sbi_remote_sfence_vma(uintptr_t hart_mask,
                          uintptr_t hart_mask_base,
                          uintptr_t start_addr,
                          uintptr_t size);
int sbi_remote_sfence_vma_asid(uintptr_t hart_mask,
                               uintptr_t hart_mask_base,
                               uintptr_t start_addr,
                               uintptr_t size,
                               uintptr_t asid);
int sbi_remote_hfence_gvma_vmid(uintptr_t hart_mask,
                                uintptr_t hart_mask_base,
                                uintptr_t start_addr,
                                uintptr_t size,
                                uintptr_t vmid);
int sbi_remote_hfence_gvma(uintptr_t hart_mask,
                           uintptr_t hart_mask_base,
                           uintptr_t start_addr,
                           uintptr_t size);
int sbi_remote_hfence_vvma_asid(uintptr_t hart_mask,
                                uintptr_t hart_mask_base,
                                uintptr_t start_addr,
                                uintptr_t size,
                                uintptr_t asid);
int sbi_remote_hfence_vvma(uintptr_t hart_mask,
                           uintptr_t hart_mask_base,
                           uintptr_t start_addr,
                           uintptr_t size);

/* HSM extension */
int sbi_hart_start(uintptr_t hartid, uintptr_t start_addr, uintptr_t opaque);
int sbi_hart_stop(void);
int sbi_hart_get_status(uintptr_t hartid, uintptr_t *status);
int sbi_hart_suspend(uint32_t suspend_type,
                     uintptr_t resume_addr,
                     uintptr_t opaque);

/* system reset extension */
int sbi_system_reset(uint32_t reset_type, uint32_t reset_reason);

/* console extension */
int sbi_debug_console_write(uintptr_t num_bytes,
                            uintptr_t base_addr_lo,
                            uintptr_t base_addr_hi);
int sbi_debug_console_read(uintptr_t num_bytes,
                           uintptr_t base_addr_lo,
                           uintptr_t base_addr_hi);
int sbi_debug_console_write_byte(uint8_t byte);

#endif
