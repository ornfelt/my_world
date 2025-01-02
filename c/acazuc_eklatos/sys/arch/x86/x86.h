#ifndef X86_H
#define X86_H

#include <types.h>
#include <time.h>
#include <irq.h>

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFF
#define MAXCPU 256

struct user_regs;
struct vm_space;
struct vm_zone;
struct page;
struct uio;

enum isa_irq_id
{
	ISA_IRQ_PIT,
	ISA_IRQ_KBD,
	ISA_IRQ_CASCADE,
	ISA_IRQ_COM2,
	ISA_IRQ_COM1,
	ISA_IRQ_LPT2,
	ISA_IRQ_FLOPPY,
	ISA_IRQ_LPT1,
	ISA_IRQ_CMOS,
	ISA_IRQ_FREE1,
	ISA_IRQ_FREE2,
	ISA_IRQ_FREE3,
	ISA_IRQ_MOUSE,
	ISA_IRQ_FPU,
	ISA_IRQ_ATA1,
	ISA_IRQ_ATA2,
};

struct user_sse
{
	uint8_t fcw;
	uint8_t fsw;
	uint8_t ftw;
	uint8_t reserved_003[1];
	uint32_t fop;
	uint32_t fip;
	uint16_t fcs;
	uint8_t reserved_00E[2];
	uint16_t fdp;
	uint16_t fds;
	uint32_t fds2;
	uint32_t mxcsr;
	uint32_t mxcsr_mask;
	uint8_t st0[10];
	uint8_t reserved_02A[6];
	uint8_t st1[10];
	uint8_t reserved_03A[6];
	uint8_t st2[10];
	uint8_t reserved_04A[6];
	uint8_t st3[10];
	uint8_t reserved_05A[6];
	uint8_t st4[10];
	uint8_t reserved_06A[6];
	uint8_t st5[10];
	uint8_t reserved_07A[6];
	uint8_t st6[10];
	uint8_t reserved_08A[6];
	uint8_t st7[10];
	uint8_t reserved_09A[6];
	uint8_t xmm0[16];
	uint8_t xmm1[16];
	uint8_t xmm2[16];
	uint8_t xmm3[16];
	uint8_t xmm4[16];
	uint8_t xmm5[16];
	uint8_t xmm6[16];
	uint8_t xmm7[16];
	uint8_t xmm8[16];
	uint8_t xmm9[16];
	uint8_t xmm10[16];
	uint8_t xmm11[16];
	uint8_t xmm12[16];
	uint8_t xmm13[16];
	uint8_t xmm14[16];
	uint8_t xmm15[16];
	uint8_t reserved_1A0[16];
	uint8_t reserved_1B0[16];
	uint8_t reserved_1C0[16];
	uint8_t reserved_1D0[16];
	uint8_t reserved_1E0[16];
	uint8_t reserved_1F0[16];
};

struct user_avx
{
	uint8_t ymm0[16];
	uint8_t ymm1[16];
	uint8_t ymm2[16];
	uint8_t ymm3[16];
	uint8_t ymm4[16];
	uint8_t ymm5[16];
	uint8_t ymm6[16];
	uint8_t ymm7[16];
	uint8_t ymm8[16];
	uint8_t ymm9[16];
	uint8_t ymm10[16];
	uint8_t ymm11[16];
	uint8_t ymm12[16];
	uint8_t ymm13[16];
	uint8_t ymm14[16];
	uint8_t ymm15[16];
};

struct user_fpu
{
	struct user_sse sse;
	uint8_t header[64];
	struct user_avx avx;
} __attribute__((aligned(64)));

void idt_init(void);
void idt_load(void);
void reload_segments(void);
void gdt_init(uint8_t cpuid);
void gdt_load(uint8_t cpuid);
int mptable_init(void);
int mptable_get_pci_irq(uint8_t bus, uint8_t slot, uint8_t pin, uint8_t *ioapic,
                        uint8_t *irq, int *active_low, int *level_trigger);
int mptable_get_isa_irq(uint8_t isa, uint8_t *ioapic, uint8_t *irq,
                        int *active_low, int *level_trigger);
void tss_set_ptr(void *ptr);
void start_ap(void);
void cpuid_load(void);
void mp_init(void);
void fpu_init(void);
void rtc_init(void);
void pit_init(void);
void pic_init(uint8_t offset1, uint8_t offset2);
void pic_enable_irq(enum isa_irq_id id);
void pic_disable_irq(enum isa_irq_id id);
void pic_eoi(enum isa_irq_id id);
void com_init(void);
void com_init_tty(void);
void tsc_init(void);
void setup_interrupt_handlers(void);
void vga_init(int rgb, uint32_t paddr, uint32_t width, uint32_t height,
              uint32_t pitch, uint32_t bpp);
void hpet_init(uint32_t hw_id, uint32_t addr, uint8_t number,
               uint16_t min_clock_ticks);

int register_pic_irq(struct irq_handle *handle, size_t id,
                     size_t cpuid, irq_fn_t fn, void *userdata);
int register_apic_irq(struct irq_handle *handle, size_t id,
                      size_t ioapic, int active_low, int level_trigger,
                      size_t cpuid, irq_fn_t fn, void *userdata);
int register_isa_irq(enum isa_irq_id id, irq_fn_t fn, void *userptr,
                     struct irq_handle *handle);

extern uint8_t g_isa_irq[16];
extern int g_has_apic;
extern struct user_fpu g_default_fpu;

#endif
