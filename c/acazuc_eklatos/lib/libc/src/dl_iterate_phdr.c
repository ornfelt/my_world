#include <link.h>

int _dl_iterate_phdr(int (*cb)(struct dl_phdr_info *info,
                               size_t size, void *data),
                     void *data);

int
dl_iterate_phdr(int (*cb)(struct dl_phdr_info *info, size_t size, void *data),
                void *data)
{
	return _dl_iterate_phdr(cb, data);
}
