#ifndef LIBFORMAT_BC1_H
# define LIBFORMAT_BC1_H

# include <cstdint>

namespace libformat
{

	class BC1
	{

	private:
		static void unpackBlock(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t *in, uint8_t *out);

	public:
		static void read(uint32_t width, uint32_t height, uint8_t *in, uint8_t *out);

	};

}

#endif
