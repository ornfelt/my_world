#include "arch/x86/x86.h"
#include "arch/x86/asm.h"

#include <cpu.h>
#include <std.h>

struct gdt_entry
{
	uint32_t base;
	uint32_t limit;
	uint8_t type;
	uint8_t flags;
};

struct gdtr
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

static const struct gdt_entry gdt_entries[] =
{
	{.base = 0, .limit = 0         , .type = 0x00, .flags = 0x40}, /* NULL */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0x9A, .flags = 0x40}, /* kern code */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0x92, .flags = 0x40}, /* kern data */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0xFA, .flags = 0x40}, /* user code */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0xF2, .flags = 0x40}, /* user data */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0x92, .flags = 0x40}, /* kern FS */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0x92, .flags = 0x40}, /* kern GS */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0xF2, .flags = 0x40}, /* user FS */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0xF2, .flags = 0x40}, /* user GS */
	{.base = 0, .limit = 0xFFFFFFFF, .type = 0xE9, .flags = 0x00}, /* TSS */
};

static void encode_entry(uint8_t *target, const struct gdt_entry *source)
{
	uint32_t limit;
	if (source->limit > 65536)
	{
		limit = source->limit >> 12;
		target[6] = source->flags | 0x80;
	}
	else
	{
		limit = source->limit;
		target[6] = source->flags;
	}

	target[0] = limit & 0xFF;
	target[1] = (limit >> 8) & 0xFF;
	target[2] = source->base & 0xFF;
	target[3] = (source->base >> 8) & 0xFF;
	target[4] = (source->base >> 16) & 0xFF;
	target[5] = source->type;
	target[6] |= (limit >> 16) & 0xF;
	target[7] = (source->base >> 24) & 0xFF;
}

void tss_set_ptr(void *ptr)
{
	curcpu()->arch.tss.esp0 = (uint32_t)ptr;
}

void gdt_init(uint8_t cpuid)
{
	struct cpu *cpu = &g_cpus[cpuid];
	for (size_t i = 0; i < sizeof(gdt_entries) / sizeof(*gdt_entries); ++i)
		encode_entry(&cpu->arch.gdt[8 * i], &gdt_entries[i]);
	struct gdt_entry cpu_entry =
	{
		.base = (uint32_t)cpu,
		.limit = sizeof(*cpu),
		.type = 0x92,
		.flags = 0x40,
	};
	encode_entry(&cpu->arch.gdt[8 * 6], &cpu_entry);
	memset(&cpu->arch.tss, 0, sizeof(cpu->arch.tss));
	cpu->arch.tss.ss0 = 0x10;
	cpu->arch.tss.esp0 = 0;
}

void gdt_load(uint8_t cpuid)
{
	struct cpu *cpu = &g_cpus[cpuid];
	/* TSS entry must be recoded to avoid busy flag */
	struct gdt_entry tss_entry =
	{
		.base = (uint32_t)&cpu->arch.tss,
		.limit = sizeof(cpu->arch.tss),
		.type = 0xE9,
		.flags = 0x00,
	};
	encode_entry(&cpu->arch.gdt[8 * 9], &tss_entry);
	struct gdtr gdtr;
	gdtr.base = (uint32_t)cpu->arch.gdt;
	gdtr.limit = sizeof(cpu->arch.gdt) - 1;
	lgdt(&gdtr);
	ltr(8 * 9);
	reload_segments();
}

void gdt_set_user_gs_base(uint32_t base)
{
	struct gdt_entry cpu_entry =
	{
		.base = (uint32_t)base,
		.limit = 0xFFFFFFFFUL - base,
		.type = 0xF2,
		.flags = 0x40,
	};
	encode_entry(&curcpu()->arch.gdt[8 * 8], &cpu_entry);
}
