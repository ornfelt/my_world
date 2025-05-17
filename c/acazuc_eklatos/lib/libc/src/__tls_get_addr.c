#include <eklat/tls.h>

#if defined(__i386__)
__attribute__((regparm(1)))
void *
___tls_get_addr(size_t *ptr)
#else
void *
__tls_get_addr(size_t *ptr)
#endif
{
	/* on risc-v (and mips, powerpc, and some other architectures)
	 * the DTV is offset by a fixed number (0x800 in the case of riscv)
	 * because the isa use signed 12-bits immediate in instructions
	 * making it easier to use the full -0x800 / +0x7FF allowed range
	 *
	 * I have a very strange feeling about this because we're not doing
	 * immediate-offset addressing in here, but the spec says offset will
	 * be there, so we're stuck with it
	 */
	size_t dtv_offset;
#if defined (__riscv) || defined(__mips__)
	dtv_offset = 0x800;
#else
	dtv_offset = 0;
#endif
	/* XXX this is racy with dlopen() in case of mods reallocation */
	return &get_tls_block()->mods[ptr[0]].data[ptr[1] + dtv_offset];
}
