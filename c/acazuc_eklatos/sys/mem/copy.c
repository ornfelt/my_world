#include <mem.h>
#include <cpu.h>

static int is_range_user(struct vm_space *vm_space, uintptr_t addr, size_t size)
{
	if (!addr)
		return 0;
	if (addr < vm_space->region.addr)
		return 0;
	uintptr_t end;
	if (__builtin_add_overflow(addr, size, &end))
		return 0;
	if (end > vm_space->region.addr + vm_space->region.size)
		return 0;
	return 1;
}

static int verify_page(struct vm_space *space, size_t addr)
{
	return arch_vm_populate_page(space, addr, VM_PROT_R, NULL);
}

int vm_verifystr(struct vm_space *space, const char *ptr)
{
	if (!is_range_user(space, (uintptr_t)ptr, 1))
		return -EFAULT;
	uintptr_t start = (uintptr_t)ptr & ~PAGE_MASK;
	uintptr_t end = space->region.addr + space->region.size;
	for (uintptr_t it = start; it < end; it += PAGE_SIZE)
	{
		int ret = verify_page(space, it);
		if (ret)
			return ret;
		uint8_t *st = (uint8_t*)it;
		size_t n = PAGE_SIZE;
		if (it == start)
		{
			size_t len = (uintptr_t)ptr - start;
			st += len;
			n -= len;
		}
		for (size_t i = 0; i < n; ++i)
		{
			if (!st[i])
				return 0;
		}
	}
	return -EFAULT;
}

int vm_verifystra(struct vm_space *space, const char * const *array)
{
	if (!is_range_user(space, (uintptr_t)array, sizeof(void*)))
		return -EFAULT;
	uintptr_t start = (uintptr_t)array & ~PAGE_MASK;
	uintptr_t end = space->region.addr + space->region.size;
	for (size_t it = start; it < end; it += PAGE_SIZE)
	{
		int ret = verify_page(space, it);
		if (ret)
			return ret;
		uint8_t *st = (uint8_t*)it;
		size_t n = PAGE_SIZE;
		if (it == start)
		{
			size_t len = (size_t)array - start;
			st += len;
			n -= len;
		}
		/* XXX we should check for unaligned ptr
		 * causing part of it to be in another page
		 */
		for (size_t i = 0; i < n / sizeof(void*); ++i)
		{
			void *str = ((void**)st)[i];
			if (!str)
				return 0;
			int res = vm_verifystr(space, str);
			if (res)
				return ret;
		}
	}
	return -EFAULT;
}

static int copyin_page(struct vm_space *space, void *kaddr,
                       const void *uaddr, size_t n)
{
	uintptr_t page = (uintptr_t)uaddr;
	uintptr_t pad = page & PAGE_MASK;
	uintptr_t len = PAGE_SIZE - pad;
	page -= pad;
	if (len > n)
		len = n;
	uintptr_t poff;
	int ret = arch_vm_populate_page(space, page, VM_PROT_R, &poff);
	if (ret)
		return ret;
	struct arch_copy_zone *zone = &curcpu()->copy_src_page;
	uint8_t *src = arch_set_copy_zone(zone, poff);
	memcpy(kaddr, &src[pad], len);
	return len;
}

int vm_copyin(struct vm_space *space, void *kaddr, const void *uaddr,
              size_t n)
{
	if (!is_range_user(space, (uintptr_t)uaddr, n))
		return -EFAULT;
	while (n)
	{
		int ret = copyin_page(space, kaddr, uaddr, n);
		if (ret < 0)
			return ret;
		kaddr = (uint8_t*)kaddr + ret;
		uaddr = (uint8_t*)uaddr + ret;
		n -= ret;
	}
	return 0;
}

static int copyout_page(struct vm_space *space, void *uaddr,
                        const void *kaddr, size_t n)
{
	uintptr_t page = (uintptr_t)uaddr;
	uintptr_t pad = page & PAGE_MASK;
	uintptr_t len = PAGE_SIZE - pad;
	page -= pad;
	if (len > n)
		len = n;
	uintptr_t poff;
	int ret = arch_vm_populate_page(space, page, VM_PROT_R, &poff);
	if (ret)
		return ret;
	struct arch_copy_zone *zone = &curcpu()->copy_dst_page;
	uint8_t *dst = arch_set_copy_zone(zone, poff);
	memcpy(&dst[pad], kaddr, len);
	return len;
}

int vm_copyout(struct vm_space *space, void *uaddr, const void *kaddr,
               size_t n)
{
	if (!is_range_user(space, (uintptr_t)uaddr, n))
		return -EFAULT;
	while (n)
	{
		int ret = copyout_page(space, uaddr, kaddr, n);
		if (ret < 0)
			return ret;
		kaddr = (uint8_t*)kaddr + ret;
		uaddr = (uint8_t*)uaddr + ret;
		n -= ret;
	}
	return 0;
}

int vm_copystr(struct vm_space *space, char *kstr, const char *ustr, size_t n)
{
	if (!is_range_user(space, (uintptr_t)ustr, 1))
		return -EFAULT;
	struct arch_copy_zone *zone = &curcpu()->copy_src_page;
	uintptr_t start = (uintptr_t)ustr & ~PAGE_MASK;
	uintptr_t end = space->region.addr + space->region.size;
	for (uintptr_t it = start; it < end; it += PAGE_SIZE)
	{
		uintptr_t poff;
		int ret = arch_vm_populate_page(space, it, VM_PROT_R, &poff);
		if (ret)
			return ret;
		uint8_t *st = arch_set_copy_zone(zone, poff);
		size_t page_n = PAGE_SIZE;
		if (it == start)
		{
			size_t len = (size_t)ustr - start;
			st += len;
			page_n -= len;
		}
		for (size_t i = 0; i < page_n; ++i)
		{
			if (!n)
				return -ENAMETOOLONG;
			*(kstr++) = st[i];
			n--;
			if (!st[i])
				return 0;
		}
	}
	return -EFAULT;
}
