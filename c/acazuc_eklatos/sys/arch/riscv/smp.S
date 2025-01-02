#if __riscv_xlen == 64
#define ll ld
#define ltype .quad
#else
#define ll lw
#define ltype .long
#define satp_page dir_page
#endif

.global smp_trampoline
.type smp_trampoline, %function
smp_trampoline:
	/* disable IRQ */
	csrci sstatus, (1 << 1)

	/* set satp & enable paging */
	ll t0, .Lsatp_page_addr
	srli t0, t0, 12
#if __riscv_xlen == 64
	li t1, 9 << 60
#else
	li t1, 1 << 31
#endif
	or t0, t0, t1
	csrw satp, t0
	sfence.vma

	ll t0, .Lg_ncpus_addr
	ll t0, (t0)
#if __riscv_xlen == 64
	slli t0, t0, 3
#else
	slli t0, t0, 2
#endif
	ll t1, .Lsmp_stacks_addr
	add t1, t1, t0
	ll sp, (t1)
	ll t0, .Lcpu_boot_addr
	jalr t0

.Lsatp_page_addr:
	ltype satp_page
.Lg_ncpus_addr:
	ltype g_ncpus
.Lsmp_stacks_addr:
	ltype smp_stacks
.Lcpu_boot_addr:
	ltype cpu_boot
