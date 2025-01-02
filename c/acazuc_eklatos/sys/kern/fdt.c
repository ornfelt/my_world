#define ENABLE_TRACE

#include <endian.h>
#include <errno.h>
#include <file.h>
#include <efi.h>
#include <fdt.h>
#include <vfs.h>
#include <uio.h>
#include <mem.h>

#define FDT_MAGIC 0xD00DFEED

#define FDT_BEGIN_NODE 0x1
#define FDT_END_NODE   0x2
#define FDT_PROP       0x3
#define FDT_NOP        0x4
#define FDT_END        0x9

struct fdt_header
{
	uint32_t magic;
	uint32_t totalsize;
	uint32_t off_dt_struct;
	uint32_t off_dt_strings;
	uint32_t off_mem_rsvmap;
	uint32_t version;
	uint32_t last_comp_version;
	uint32_t boot_cpuid_phys;
	uint32_t size_dt_strings;
	uint32_t size_dt_struct;
};

struct fdt_parser
{
	uint32_t begin;
	uint32_t end;
	uint32_t size;
	uint32_t pos;
};

static uint8_t *fdt_data;
static struct fdt_header *fdt_header;
struct fdt_node_head fdt_nodes = TAILQ_HEAD_INITIALIZER(fdt_nodes);
static struct node *fdt_node;

static int get_fdt_size(uintptr_t addr, size_t *sizep)
{
	struct fdt_header *header;
	uint8_t *ptr = NULL;
	size_t size;
	struct page *pages[2];
	int ret;

	size = PAGE_SIZE;
	if (PAGE_SIZE - addr % PAGE_SIZE >= PAGE_SIZE)
		size += PAGE_SIZE;
	ret = pm_fetch_pages(addr / PAGE_SIZE, size / PAGE_SIZE, pages);
	if (ret)
	{
		struct page direct_page;
		pm_init_page(&direct_page, addr / PAGE_SIZE);
		ptr = vm_map(&direct_page, size, VM_PROT_R);
	}
	else
	{
		ptr = vm_map_pages(pages, size / PAGE_SIZE, VM_PROT_R);
		for (size_t i = 0; i < size / PAGE_SIZE; ++i)
			pm_free_page(pages[i]);
	}
	if (!ptr)
	{
		TRACE("fdt: failed to map table");
		ret = -ENOMEM;
		goto err;
	}
	header = (struct fdt_header*)(ptr + addr % PAGE_SIZE);
	if (header->magic != be32toh(FDT_MAGIC))
	{
		TRACE("fdt: invalid magic: 0x%" PRIx32, header->magic);
		ret = -EXDEV;
		goto err;
	}
	*sizep = be32toh(header->totalsize) + addr % PAGE_SIZE;
	*sizep += PAGE_SIZE - 1;
	*sizep -= *sizep % PAGE_SIZE;
	vm_unmap(ptr, size);
	return 0;

err:
	if (ptr)
		vm_unmap(ptr, size);
	return ret;
}

static int map_fdt(void)
{
	uintptr_t fdt_addr;
	size_t fdt_size = 0;
	struct page **pages = NULL;
	int ret;

	fdt_addr = efi_get_fdt();
	if (!fdt_addr)
	{
		TRACE("fdt: no efi FDT addr");
		return -ENOENT;
	}
	ret = get_fdt_size(fdt_addr, &fdt_size);
	if (ret)
		goto end;
	pages = malloc(sizeof(*pages) * fdt_size / PAGE_SIZE, M_ZERO);
	if (!pages)
	{
		TRACE("fdt: failed to allocate pages array");
		ret = -ENOMEM;
		goto end;
	}
	ret = pm_fetch_pages(fdt_addr / PAGE_SIZE, fdt_size / PAGE_SIZE, pages);
	if (ret)
	{
		struct page direct_page;
		pm_init_page(&direct_page, fdt_addr / PAGE_SIZE);
		fdt_data = vm_map(&direct_page, fdt_size, VM_PROT_R);
	}
	else
	{
		fdt_data = vm_map_pages(pages, fdt_size / PAGE_SIZE, VM_PROT_R);
	}
	if (!fdt_data)
	{
		TRACE("fdt: failed to map fdt");
		ret = -ENOMEM;
		goto end;
	}
	fdt_header = (struct fdt_header*)(fdt_data + fdt_addr % PAGE_SIZE);
	ret = 0;

end:
	if (ret)
	{
		if (fdt_data)
			vm_unmap(fdt_data, fdt_size);
	}
	if (pages)
	{
		for (size_t i = 0; i < fdt_size / PAGE_SIZE; ++i)
			pm_free_page(pages[i]);
		free(pages);
	}
	return ret;
}

static int parser_getu8(struct fdt_parser *parser, uint8_t *val)
{
	uint32_t next;
	if (__builtin_add_overflow(parser->pos, 1, &next))
	{
		TRACE("fdt: parser position overflow");
		return -EOVERFLOW;
	}
	if (next > parser->end)
	{
		TRACE("fdt: parser out of bounds: %" PRIu32 " / %" PRIu32,
		      next, parser->end);
		return -EOVERFLOW;
	}
	*val = fdt_data[parser->pos];
	parser->pos = next;
	return 0;
}

static int parser_getu32(struct fdt_parser *parser, uint32_t *val)
{
	uint32_t next;
	if (__builtin_add_overflow(parser->pos, 4, &next))
	{
		TRACE("fdt: parser position overflow");
		return -EOVERFLOW;
	}
	if (next > parser->end)
	{
		TRACE("fdt: parser out of bounds: %" PRIu32 " / %" PRIu32,
		      next, parser->end);
		return -EOVERFLOW;
	}
	*val = be32toh(*(uint32_t*)&fdt_data[parser->pos]);
	parser->pos = next;
	return 0;
}

static int parser_align(struct fdt_parser *parser)
{
	while (parser->pos % 4)
	{
		uint8_t c;
		int ret = parser_getu8(parser, &c);
		if (ret)
			return ret;
#if 0
		if (c)
			TRACE("fdt: padding not null: 0x%" PRIx8, c);
#endif
	}
	return 0;
}

static void free_node(struct fdt_node *node)
{
	if (!node)
		return;
	struct fdt_node *child;
	struct fdt_node *next_child;
	TAILQ_FOREACH_SAFE(child, &node->children, chain, next_child)
		free_node(child);
	struct fdt_prop *prop;
	struct fdt_prop *next_prop;
	TAILQ_FOREACH_SAFE(prop, &node->properties, chain, next_prop)
		free(prop);
	free(node->name);
	free(node);
}

static int parse_node(struct fdt_parser *parser, struct fdt_node **nodep)
{
	struct fdt_node *node = NULL;
	uint32_t token;
	uint32_t name_start;
	int ret;

	node = malloc(sizeof(*node), M_ZERO);
	if (!node)
	{
		TRACE("fdt: node allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	TAILQ_INIT(&node->children);
	TAILQ_INIT(&node->properties);
	name_start = parser->pos;
	while (1)
	{
		uint8_t c;
		ret = parser_getu8(parser, &c);
		if (ret)
			goto err;
		if (!c)
			break;
	}
	node->name = malloc(parser->pos - name_start, 0);
	if (!node->name)
	{
		TRACE("fdt: node name allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	memcpy(node->name, &fdt_data[name_start], parser->pos - name_start);
	node->name[parser->pos - name_start - 1] = '\0';
	ret = parser_align(parser);
	if (ret)
		goto err;
	while (1)
	{
		ret = parser_getu32(parser, &token);
		if (ret)
			goto err;
		if (token == FDT_END_NODE)
			break;
		switch (token)
		{
			case FDT_NOP:
				break;
			case FDT_PROP:
			{
				uint32_t len;
				uint32_t nameoff;
				ret = parser_getu32(parser, &len);
				if (ret)
					goto err;
				ret = parser_getu32(parser, &nameoff);
				if (ret)
					goto err;
				struct fdt_prop *prop = malloc(sizeof(*prop) + len, 0);
				if (!prop)
				{
					TRACE("fdt: property allocation failed");
					ret = -ENOMEM;
					goto err;
				}
				prop->len = len;
				/* XXX bound check */
				prop->name = (char*)&fdt_data[be32toh(fdt_header->off_dt_strings) + nameoff];
				for (size_t i = 0; i < len; ++i)
				{
					ret = parser_getu8(parser, &prop->data[i]);
					if (ret)
						goto err;
				}
				TAILQ_INSERT_TAIL(&node->properties, prop, chain);
				ret = parser_align(parser);
				if (ret)
					goto err;
				break;
			}
			case FDT_BEGIN_NODE:
			{
				struct fdt_node *child;
				ret = parse_node(parser, &child);
				if (ret)
					goto err;
				child->parent = node;
				TAILQ_INSERT_TAIL(&node->children, child, chain);
				break;
			}
			case FDT_END:
				TRACE("fdt: invalid FDT_END");
				ret = -EINVAL;
				goto err;
			default:
				TRACE("fdt: unknown token 0x%" PRIx32, token);
				ret = -EINVAL;
				goto err;
		}
	}
	*nodep = node;
	return 0;

err:
	free_node(node);
	return ret;
}

static void print_node(struct uio *uio, struct fdt_node *node, size_t indent)
{
	static const char *tabs = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	uprintf(uio, "%.*s%s\n", (int)indent, tabs, node->name);
	struct fdt_prop *prop;
	TAILQ_FOREACH(prop, &node->properties, chain)
		uprintf(uio, "%.*s%s: %" PRIu32 " bytes\n",
		        (int)indent + 1, tabs, prop->name, prop->len);
	struct fdt_node *child;
	TAILQ_FOREACH(child, &node->children, chain)
		print_node(uio, child, indent + 1);
}

static ssize_t fdt_fread(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	struct fdt_node *child;
	TAILQ_FOREACH(child, &fdt_nodes, chain)
		print_node(uio, child, 0);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static struct file_op fdt_fop =
{
	.read = fdt_fread,
};

int fdt_init(void)
{
	int ret = map_fdt();
	if (ret)
		return ret;
	struct fdt_parser parser;
	parser.begin = be32toh(fdt_header->off_dt_struct);
	parser.size = be32toh(fdt_header->size_dt_struct);
	if (__builtin_add_overflow(parser.begin, parser.size, &parser.end))
	{
		TRACE("fdt: invalid struct size");
		return -EINVAL;
	}
	if (parser.end > be32toh(fdt_header->totalsize))
	{
		TRACE("fdt: struct out of bounds");
		return -EINVAL;
	}
	parser.pos = parser.begin;
	while (parser.pos < parser.end)
	{
		uint32_t token;
		if (parser_getu32(&parser, &token))
		{
			TRACE("fdt: token eof");
			return -EINVAL;
		}
		if (token == FDT_NOP)
			continue;
		if (token == FDT_END)
		{
			if (parser.pos != parser.end)
				TRACE("fdt: FDT_END not at the end");
			break;
		}
		if (token != FDT_BEGIN_NODE)
		{
			TRACE("fdt: invalid root token: 0x%08" PRIx32, token);
			return -EINVAL;
		}
		struct fdt_node *node;
		if (parse_node(&parser, &node))
		{
			TRACE("fdt: failed to parse node");
			return -EINVAL;
		}
		TAILQ_INSERT_TAIL(&fdt_nodes, node, chain);
	}
	ret = sysfs_mknode("fdt", 0, 0, 0400, &fdt_fop, &fdt_node);
	if (ret)
		printf("fdt: failed to create fdt sysnode: %s\n", strerror(ret));
	return 0;
}

struct fdt_prop *fdt_get_prop(const struct fdt_node *node, const char *name)
{
	struct fdt_prop *prop;
	TAILQ_FOREACH(prop, &node->properties, chain)
	{
		if (!strcmp(prop->name, name))
			return prop;
	}
	return NULL;
}

int fdt_get_base_size_reg(const struct fdt_prop *prop, size_t id,
                          uintptr_t *basep, size_t *sizep)
{
	if (prop->len < (id + 1) * 16)
	{
		TRACE("fdt: register out of bounds");
		return -EINVAL;
	}
	uint64_t base = be64toh(*(uint64_t*)&prop->data[id * 16 + 0]);
	uint64_t size = be64toh(*(uint64_t*)&prop->data[id * 16 + 8]);
#if __SIZE_WIDTH__ == 32
	if (base >> 32)
	{
		TRACE("fdt: base out of bounds");
		return -EOVERFLOW;
	}
	if (size >> 32)
	{
		TRACE("fdt: size out of bounds");
		return -EOVERFLOW;
	}
#endif
	if (base % PAGE_SIZE)
	{
		TRACE("fdt: unaligned base: 0x%" PRIx64, base);
		return -EINVAL;
	}
	/* XXX
	 * we should somehow check for a valid size,
	 * the previously PAGE_SIZE minimum bound is not valid
	 * for some devices (e.g.: uart can get 100 bytes)
	 */
	if (!size)
	{
		TRACE("fdt: invalid size: 0x%" PRIx64, size);
		return -EINVAL;
	}
	*basep = base;
	*sizep = size;
	return 0;
}

static int get_ecam_addr_node(uint8_t bus, uint8_t slot, uint8_t func,
                              const struct fdt_node *node, uintptr_t *poffp)
{
	struct fdt_node *child;
	TAILQ_FOREACH(child, &node->children, chain)
	{
		if (!fdt_check_compatible(child, "simple-bus"))
		{
			if (!get_ecam_addr_node(bus, slot, func, child, poffp))
				return 0;
			continue;
		}
		if (fdt_check_compatible(child, "pci-host-ecam-generic"))
			continue;
		const struct fdt_prop *reg = fdt_get_prop(child, "reg");
		if (!reg)
			continue;
		uintptr_t mmio_base;
		size_t mmio_size;
		int ret = fdt_get_base_size_reg(reg, 0, &mmio_base,
		                                &mmio_size);
		if (ret)
			continue;
		struct fdt_prop *bus_range = fdt_get_prop(child, "bus-range");
		if (!bus_range)
			continue;
		if (bus_range->len != 8)
			continue;
		uint32_t bus_start = be32toh(*(uint32_t*)&bus_range->data[0]);
		uint32_t bus_end = be32toh(*(uint32_t*)&bus_range->data[4]);
		if (bus < bus_start
		 || bus >= bus_end)
			continue;
		uintptr_t poff = 0;
		poff |= ((bus - bus_start) << 8);
		poff |= (slot << 3);
		poff |= (func << 0);
		if (poff * PAGE_SIZE >= mmio_size)
			continue;
		*poffp = poff + mmio_base / PAGE_SIZE;
		return 0;
	}
	return -ENOENT;
}

int fdt_get_ecam_addr(uint8_t bus, uint8_t slot, uint8_t func,
                      uintptr_t *poffp)
{
	const struct fdt_node *node;
	TAILQ_FOREACH(node, &fdt_nodes, chain)
	{
		if (!get_ecam_addr_node(bus, slot, func, node, poffp))
			return 0;
	}
	return -ENOENT;
}

int fdt_check_compatible(const struct fdt_node *node, const char *str)
{
	const struct fdt_prop *compat = fdt_get_prop(node, "compatible");
	if (!compat)
		return -EINVAL;
	if (!compat->len)
		return -EINVAL;
	size_t str_len = strlen(str);
	for (size_t i = 0; i < compat->len;)
	{
		size_t len = strnlen((char*)&compat->data[i], compat->len - i);
		if (len == str_len)
		{
			if (!memcmp((char*)&compat->data[i], str, len))
				return 0;
		}
		i += len + 1;
	}
	return -EINVAL;
}

static int find_phandle_node(uint32_t phandle, const struct fdt_node *node,
                             struct fdt_node **nodep)
{
	const struct fdt_node *child;
	TAILQ_FOREACH(child, &node->children, chain)
	{
		if (!fdt_check_compatible(child, "simple-bus"))
		{
			if (!find_phandle_node(phandle, child, nodep))
				return 0;
			continue;
		}
		const struct fdt_prop *phandle_prop = fdt_get_prop(child,
		                                                   "phandle");
		if (!phandle_prop)
			continue;
		if (phandle_prop->len != 4)
			continue;
		if (be32toh(*(uint32_t*)phandle_prop->data) != phandle)
			continue;
		*nodep = (struct fdt_node*)child;
		return 0;
	}
	return -ENOENT;
}

int fdt_find_phandle(uint32_t phandle, struct fdt_node **nodep)
{
	struct fdt_node *node;
	TAILQ_FOREACH(node, &fdt_nodes, chain)
	{
		if (!find_phandle_node(phandle, node, nodep))
			return 0;
	}
	return -ENOENT;
}
