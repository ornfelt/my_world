#include "arch/x86/x86.h"
#include "arch/x86/msr.h"
#include "arch/x86/asm.h"

#include <cpu.h>
#include <std.h>

struct gdt_entry
{
	uint64_t base;
	uint32_t limit;
	uint8_t type;
	uint8_t flags;
};

struct gdtr
{
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

static const struct gdt_entry gdt_entries[] =
{
	{.base = 0, .limit = 0         , .type = 0x00, .flags = 0x20}, /* NULL */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0x9A, .flags = 0x20}, /* kern code */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0x92, .flags = 0x20}, /* kern data */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0xFA, .flags = 0x40}, /* user code 32 */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0xF2, .flags = 0x40}, /* user data 32/64 */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0xFA, .flags = 0x20}, /* user code 64 */
};

static void encode_entry(uint8_t *target, const struct gdt_entry *source)
{
	uint32_t limit;
	if (source->limit > 65536)
	{
		limit = source->limit >> 12;
		target[0x6] = source->flags | 0x80;
	}
	else
	{
		limit = source->limit;
		target[0x6] = source->flags;
	}

	target[0x0] = limit & 0xFF;
	target[0x1] = (limit >> 8) & 0xFF;
	target[0x2] = source->base & 0xFF;
	target[0x3] = (source->base >> 8) & 0xFF;
	target[0x4] = (source->base >> 16) & 0xFF;
	target[0x5] = source->type;
	target[0x6] |= (limit >> 16) & 0xF;
	target[0x7] = (source->base >> 24) & 0xFF;
	target[0x8] = (source->base >> 32) & 0xFF;
	target[0x9] = (source->base >> 40) & 0xFF;
	target[0xA] = (source->base >> 48) & 0xFF;
	target[0xB] = (source->base >> 56) & 0xFF;
	target[0xC] = 0;
	target[0xD] = 0;
	target[0xE] = 0;
	target[0xF] = 0;
}

void tss_set_ptr(void *ptr)
{
	curcpu()->arch.tss.rsp0 = (uint64_t)ptr;
}

void gdt_init(uint8_t cpuid)
{
	struct cpu *cpu = &g_cpus[cpuid];
	for (size_t i = 0; i < sizeof(gdt_entries) / sizeof(*gdt_entries); ++i)
		encode_entry(&cpu->arch.gdt[8 * i], &gdt_entries[i]);
	struct gdt_entry tss_entry =
	{
		.base = (uint64_t)&cpu->arch.tss,
		.limit = sizeof(cpu->arch.tss),
		.type = 0xE9,
		.flags = 0x00,
	};
	encode_entry(&cpu->arch.gdt[8 * 7], &tss_entry);
	memset(&cpu->arch.tss, 0, sizeof(cpu->arch.tss));
	cpu->arch.tss.rsp0 = 0;
}

void gdt_load(uint8_t cpuid)
{
	struct cpu *cpu = &g_cpus[cpuid];
	struct gdtr gdtr;
	gdtr.base = (uint64_t)&cpu->arch.gdt[0];
	gdtr.limit = sizeof(cpu->arch.gdt) - 1;
	lgdt(&gdtr);
	ltr(8 * 7);
	reload_segments();
	wrmsr(MSR_GS_BASE, (uintptr_t)cpu);
	wrmsr(MSR_KERNEL_GS_BASE, (uintptr_t)cpu);
}
