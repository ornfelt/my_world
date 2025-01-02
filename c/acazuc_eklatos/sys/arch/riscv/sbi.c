#include "arch/riscv/sbi.h"

#include <errno.h>
#include <std.h>

#define SBI_SUCCESS               (-0)
#define SBI_ERR_FAILED            (-1)
#define SBI_ERR_NOT_SUPPORTED     (-2)
#define SBI_ERR_INVALID_PARAM     (-3)
#define SBI_ERR_DENIED            (-4)
#define SBI_ERR_INVALID_ADDRESS   (-5)
#define SBI_ERR_ALREADY_AVAILABLE (-6)
#define SBI_ERR_ALREADY_STARTED   (-7)
#define SBI_ERR_ALREADY_STOPPED   (-8)
#define SBI_ERR_NO_SHMEM          (-9)
#define SBI_ERR_INVALID_STATE     (-10)
#define SBI_ERR_BAD_RANGE         (-11)
#define SBI_ERR_TIMEOUT           (-12)
#define SBI_ERR_IO                (-13)

static int sbi_ret(long r_err, unsigned long r_val, uintptr_t *ret)
{
	switch (r_err)
	{
		case SBI_SUCCESS:
			if (ret)
				*ret = r_val;
			return 0;
		case SBI_ERR_FAILED:
			return -EINVAL; /* XXX */
		case SBI_ERR_NOT_SUPPORTED:
			return -ENOSYS;
		case SBI_ERR_INVALID_PARAM:
			return -EINVAL;
		case SBI_ERR_DENIED:
			return -EPERM;
		case SBI_ERR_INVALID_ADDRESS:
			return -EFAULT;
		case SBI_ERR_ALREADY_AVAILABLE:
			return -EIO; /* XXX */
		case SBI_ERR_ALREADY_STARTED:
			return -EIO; /* XXX */
		case SBI_ERR_ALREADY_STOPPED:
			return -EIO; /* XXX */
		case SBI_ERR_NO_SHMEM:
			return -ENOENT; /* XXX */
		case SBI_ERR_INVALID_STATE:
			return -ENOENT; /* XXX */
		case SBI_ERR_BAD_RANGE:
			return -ERANGE;
		case SBI_ERR_TIMEOUT:
			return -ETIMEDOUT;
		case SBI_ERR_IO:
			return -EIO;
		default:
			return -EINVAL; /* XXX */
	}
}

static int sbi_call0(uintptr_t eid, uintptr_t fid, uintptr_t *ret)
{
	register unsigned long r_eid __asm__ ("a7") = eid;
	register unsigned long r_fid __asm__ ("a6") = fid;
	register unsigned long r_err __asm__ ("a0");
	register unsigned long r_val __asm__ ("a1");
	__asm__ volatile ("ecall":
	                  "=r"(r_err),
	                  "=r"(r_val):
	                  "r"(r_eid),
	                  "r"(r_fid));
	return sbi_ret(r_err, r_val, ret);
}

static int sbi_call1(uintptr_t eid, uintptr_t fid,
                     uintptr_t a0,
                     uintptr_t *ret)
{
	register unsigned long r_eid __asm__ ("a7") = eid;
	register unsigned long r_fid __asm__ ("a6") = fid;
	register unsigned long r_a0 __asm__ ("a0") = a0;
	register unsigned long r_err __asm__ ("a0");
	register unsigned long r_val __asm__ ("a1");
	__asm__ volatile ("ecall":
	                  "=r"(r_err),
	                  "=r"(r_val):
	                  "r"(r_eid),
	                  "r"(r_fid),
	                  "r"(r_a0));
	return sbi_ret(r_err, r_val, ret);
}

static int sbi_call2(uintptr_t eid, uintptr_t fid,
                     uintptr_t a0, uintptr_t a1,
                     uintptr_t *ret)
{
	register unsigned long r_eid __asm__ ("a7") = eid;
	register unsigned long r_fid __asm__ ("a6") = fid;
	register unsigned long r_a0 __asm__ ("a0") = a0;
	register unsigned long r_a1 __asm__ ("a1") = a1;
	register unsigned long r_err __asm__ ("a0");
	register unsigned long r_val __asm__ ("a1");
	__asm__ volatile ("ecall":
	                  "=r"(r_err),
	                  "=r"(r_val):
	                  "r"(r_eid),
	                  "r"(r_fid),
	                  "r"(r_a0),
	                  "r"(r_a1));
	return sbi_ret(r_err, r_val, ret);
}

static int sbi_call3(uintptr_t eid, uintptr_t fid,
                     uintptr_t a0, uintptr_t a1,
                     uintptr_t a2,
                     uintptr_t *ret)
{
	register unsigned long r_eid __asm__ ("a7") = eid;
	register unsigned long r_fid __asm__ ("a6") = fid;
	register unsigned long r_a0 __asm__ ("a0") = a0;
	register unsigned long r_a1 __asm__ ("a1") = a1;
	register unsigned long r_a2 __asm__ ("a2") = a2;
	register unsigned long r_err __asm__ ("a0");
	register unsigned long r_val __asm__ ("a1");
	__asm__ volatile ("ecall":
	                  "=r"(r_err),
	                  "=r"(r_val):
	                  "r"(r_eid),
	                  "r"(r_fid),
	                  "r"(r_a0),
	                  "r"(r_a1),
	                  "r"(r_a2));
	return sbi_ret(r_err, r_val, ret);
}

static int sbi_call4(uintptr_t eid, uintptr_t fid,
                     uintptr_t a0, uintptr_t a1,
                     uintptr_t a2, uintptr_t a3,
                     uintptr_t *ret)
{
	register unsigned long r_eid __asm__ ("a7") = eid;
	register unsigned long r_fid __asm__ ("a6") = fid;
	register unsigned long r_a0 __asm__ ("a0") = a0;
	register unsigned long r_a1 __asm__ ("a1") = a1;
	register unsigned long r_a2 __asm__ ("a2") = a2;
	register unsigned long r_a3 __asm__ ("a3") = a3;
	register unsigned long r_err __asm__ ("a0");
	register unsigned long r_val __asm__ ("a1");
	__asm__ volatile ("ecall":
	                  "=r"(r_err),
	                  "=r"(r_val):
	                  "r"(r_eid),
	                  "r"(r_fid),
	                  "r"(r_a0),
	                  "r"(r_a1),
	                  "r"(r_a2),
	                  "r"(r_a3));
	return sbi_ret(r_err, r_val, ret);
}

static int sbi_call5(uintptr_t eid, uintptr_t fid,
                     uintptr_t a0, uintptr_t a1,
                     uintptr_t a2, uintptr_t a3,
                     uintptr_t a4,
                     uintptr_t *ret)
{
	register unsigned long r_eid __asm__ ("a7") = eid;
	register unsigned long r_fid __asm__ ("a6") = fid;
	register unsigned long r_a0 __asm__ ("a0") = a0;
	register unsigned long r_a1 __asm__ ("a1") = a1;
	register unsigned long r_a2 __asm__ ("a2") = a2;
	register unsigned long r_a3 __asm__ ("a3") = a3;
	register unsigned long r_a4 __asm__ ("a4") = a4;
	register unsigned long r_err __asm__ ("a0");
	register unsigned long r_val __asm__ ("a1");
	__asm__ volatile ("ecall":
	                  "=r"(r_err),
	                  "=r"(r_val):
	                  "r"(r_eid),
	                  "r"(r_fid),
	                  "r"(r_a0),
	                  "r"(r_a1),
	                  "r"(r_a2),
	                  "r"(r_a3),
	                  "r"(r_a4));
	return sbi_ret(r_err, r_val, ret);
}

int sbi_get_spec_version(uintptr_t *spec_version)
{
	return sbi_call0(0x10, 0x0, spec_version);
}

int sbi_get_impl_id(uintptr_t *impl_id)
{
	return sbi_call0(0x10, 0x1, impl_id);
}

int sbi_get_impl_version(uintptr_t *impl_version)
{
	return sbi_call0(0x10, 0x2, impl_version);
}

int sbi_probe_extension(uintptr_t ext_id, uintptr_t *implemented)
{
	return sbi_call1(0x10, 0x3, ext_id, implemented);
}

int sbi_get_mvendorid(uintptr_t *mvendorid)
{
	return sbi_call0(0x10, 0x4, mvendorid);
}

int sbi_get_marchid(uintptr_t *marchid)
{
	return sbi_call0(0x10, 0x5, marchid);
}

int sbi_get_mimpid(uintptr_t *mimpid)
{
	return sbi_call0(0x10, 0x6, mimpid);
}

int sbi_set_timer(uint64_t stime_value)
{
#if __riscv_xlen == 64
	return sbi_call1(SBI_EXT_TIMER, 0x0, stime_value, NULL);
#else
	return sbi_call2(SBI_EXT_TIMER, 0x0, stime_value >> 32, stime_value, NULL);
#endif
}

int sbi_send_ipi(uintptr_t hart_mask, uintptr_t hart_mask_base)
{
	return sbi_call2(SBI_EXT_IPI, 0x0, hart_mask, hart_mask_base, NULL);
}

int sbi_remote_fence_i(uintptr_t hart_mask, uintptr_t hart_mask_base)
{
	return sbi_call2(SBI_EXT_RFENCE, 0x0, hart_mask, hart_mask_base, NULL);
}

int sbi_remote_sfence_vma(uintptr_t hart_mask,
                          uintptr_t hart_mask_base,
                          uintptr_t start_addr,
                          uintptr_t size)
{
	return sbi_call4(SBI_EXT_RFENCE, 0x1, hart_mask, hart_mask_base,
	                 start_addr, size, NULL);
}
int sbi_remote_sfence_vma_asid(uintptr_t hart_mask,
                               uintptr_t hart_mask_base,
                               uintptr_t start_addr,
                               uintptr_t size,
                               uintptr_t asid)
{
	return sbi_call5(SBI_EXT_RFENCE, 0x2, hart_mask, hart_mask_base,
	                 start_addr, size, asid, NULL);
}

int sbi_remote_hfence_gvma_vmid(uintptr_t hart_mask,
                                uintptr_t hart_mask_base,
                                uintptr_t start_addr,
                                uintptr_t size,
                                uintptr_t vmid)
{
	return sbi_call5(SBI_EXT_RFENCE, 0x3, hart_mask, hart_mask_base,
	                 start_addr, size, vmid, NULL);
}

int sbi_remote_hfence_gvma(uintptr_t hart_mask,
                           uintptr_t hart_mask_base,
                           uintptr_t start_addr,
                           uintptr_t size)
{
	return sbi_call4(SBI_EXT_RFENCE, 0x4, hart_mask, hart_mask_base,
	                 start_addr, size, NULL);
}

int sbi_remote_hfence_vvma_asid(uintptr_t hart_mask,
                                uintptr_t hart_mask_base,
                                uintptr_t start_addr,
                                uintptr_t size,
                                uintptr_t asid)
{
	return sbi_call5(SBI_EXT_RFENCE, 0x5, hart_mask, hart_mask_base,
	                 start_addr, size, asid, NULL);
}

int sbi_remote_hfence_vvma(uintptr_t hart_mask,
                           uintptr_t hart_mask_base,
                           uintptr_t start_addr,
                           uintptr_t size)
{
	return sbi_call4(SBI_EXT_RFENCE, 0x6, hart_mask, hart_mask_base,
	                 start_addr, size, NULL);
}

int sbi_hart_start(uintptr_t hartid, uintptr_t start_addr, uintptr_t opaque)
{
	return sbi_call3(SBI_EXT_HSM, 0x0, hartid, start_addr, opaque, NULL);
}

int sbi_hart_stop(void)
{
	return sbi_call0(SBI_EXT_HSM, 0x1, NULL);
}

int sbi_hart_get_status(uintptr_t hartid, uintptr_t *status)
{
	return sbi_call1(SBI_EXT_HSM, 0x2, hartid, status);
}

int sbi_hart_suspend(uint32_t suspend_type, uintptr_t resume_addr,
                     uintptr_t opaque)
{
	return sbi_call3(SBI_EXT_HSM, 0x3, suspend_type, resume_addr, opaque, NULL);
}

int sbi_system_reset(uint32_t reset_type, uint32_t reset_reason)
{
	return sbi_call2(SBI_EXT_RESET, 0x0, reset_type, reset_reason, NULL);
}

int sbi_debug_console_write(uintptr_t num_bytes,
                            uintptr_t base_addr_lo,
                            uintptr_t base_addr_hi)
{
	return sbi_call3(SBI_EXT_CONSOLE, 0x0, num_bytes, base_addr_lo,
	                 base_addr_hi, NULL);
}

int sbi_debug_console_read(uintptr_t num_bytes,
                           uintptr_t base_addr_lo,
                           uintptr_t base_addr_hi)
{
	return sbi_call3(SBI_EXT_CONSOLE, 0x0, num_bytes, base_addr_lo,
	                 base_addr_hi, NULL);
}

int sbi_debug_console_write_byte(uint8_t byte)
{
	return sbi_call1(SBI_EXT_CONSOLE, 0x2, byte, NULL);
}
