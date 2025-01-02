#include <multiboot.h>
#include <file.h>
#include <uio.h>
#include <vfs.h>
#include <std.h>

uint8_t *multiboot_ptr;

const struct multiboot_tag *multiboot_find_tag(uint16_t type)
{
	const uint8_t *ptr = &multiboot_ptr[8];
	while (1)
	{
		const struct multiboot_tag *tag = (struct multiboot_tag*)ptr;
		if (!tag->type || !tag->size)
			return NULL;
		if (tag->type == type)
			return tag;
		ptr += ((tag->size + 7) & ~7);
	}
}

void multiboot_iterate_memory(uintptr_t min, uintptr_t max,
                              void (*cb)(uintptr_t addr, size_t size, void *userdata),
                              void *userdata)
{
	const struct multiboot_tag *mb_tag = multiboot_find_tag(MULTIBOOT_TAG_TYPE_MMAP);
	if (!mb_tag)
		panic("multiboot gave no memory maps\n");
	for (size_t i = 0; i < mb_tag->size - 16;
	     i += ((struct multiboot_tag_mmap*)mb_tag)->entry_size)
	{
		const multiboot_memory_map_t *mmap = (multiboot_memory_map_t*)((uint8_t*)((struct multiboot_tag_mmap*)mb_tag)->entries + i);
		if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE)
			continue;
		if (mmap->addr >= max)
			continue;
		uint64_t end;
		if (__builtin_add_overflow(mmap->addr, mmap->len, &end))
			continue;
		if (end < min)
			continue;
		if (end > max)
			end = max;
		uintptr_t addr = mmap->addr;
		size_t size = end - mmap->addr;
		if (addr < min)
		{
			size -= min - addr;
			addr = min;
		}
		cb(addr, size, userdata);
	}
}

static const char *tag_name(uint32_t type)
{
	static const char *names[] =
	{
#define TAG_NAME_DEF(tag) [MULTIBOOT_TAG_TYPE_##tag] = #tag
		TAG_NAME_DEF(END),
		TAG_NAME_DEF(CMDLINE),
		TAG_NAME_DEF(BOOT_LOADER_NAME),
		TAG_NAME_DEF(MODULE),
		TAG_NAME_DEF(BASIC_MEMINFO),
		TAG_NAME_DEF(BOOTDEV),
		TAG_NAME_DEF(MMAP),
		TAG_NAME_DEF(VBE),
		TAG_NAME_DEF(FRAMEBUFFER),
		TAG_NAME_DEF(ELF_SECTIONS),
		TAG_NAME_DEF(APM),
		TAG_NAME_DEF(EFI32),
		TAG_NAME_DEF(EFI64),
		TAG_NAME_DEF(SMBIOS),
		TAG_NAME_DEF(ACPI_OLD),
		TAG_NAME_DEF(ACPI_NEW),
		TAG_NAME_DEF(NETWORK),
		TAG_NAME_DEF(EFI_MMAP),
		TAG_NAME_DEF(EFI_BS),
		TAG_NAME_DEF(EFI32_IH),
		TAG_NAME_DEF(EFI64_IH),
		TAG_NAME_DEF(LOAD_BASE_ADDR),
		TAG_NAME_DEF(ELF64_SECTIONS),
	};
	if (type >= sizeof(names) / sizeof(*names) || !names[type])
		return "unknown";
	return names[type];
}

static int print_multiboot(struct uio *uio)
{
	const uint8_t *ptr = &multiboot_ptr[8];
	while (1)
	{
		const struct multiboot_tag *tag = (struct multiboot_tag*)ptr;
		if (!tag->type || !tag->size)
			break;
		uprintf(uio, "%s (%lu bytes)\n", tag_name(tag->type),
		        (unsigned long)tag->size);
		ptr += ((tag->size + 7) & ~7);
	}
	return 0;
}

static ssize_t multiboot_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	int ret = print_multiboot(uio);
	if (ret < 0)
		return ret;
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op multiboot_fop =
{
	.read = multiboot_read,
};

int multiboot_register_sysfs(void)
{
	return sysfs_mknode("multiboot", 0, 0, 0444, &multiboot_fop, NULL);
}
