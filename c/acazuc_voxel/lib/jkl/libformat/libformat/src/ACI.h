#ifndef LIB_FORMAT_ACI_H
# define LIB_FORMAT_ACI_H

# include <cstdint>
# include <string>
# include <vector>

namespace libformat
{

	enum ACIFormat
	{
		ACI_BGRA8 = 1,
		ACI_RG8 = 2,
		ACI_R8 = 3,
		ACI_BGR565 = 4,
		ACI_BGRA4 = 5,
		ACI_BGR5A1 = 6,

		ACI_BC1 = 10,
		ACI_BC2 = 11,
		ACI_BC3 = 12,
		ACI_BC4 = 13,
		ACI_BC5 = 14,
		ACI_BC6 = 15,
		ACI_BC7 = 16,
	};

	enum ACICompressionAlgorithm
	{
		ACI_COMPRESSION_NONE = 1,
		ACI_COMPRESSION_ZLIB = 2,
	};

	struct ACIHeader
	{
		uint32_t magic;
		uint8_t version;
		uint8_t mipmaps;
		uint8_t format;
		uint8_t compression;
		uint32_t width;
		uint32_t height;
	};

	class ACI
	{

		public:
			static bool read(std::string file, enum ACIFormat &format, std::vector<std::vector<uint8_t>> &mipmaps, uint32_t &width, uint32_t &height);
			static bool write(std::string file, enum ACICompressionAlgorithm algorithm, enum ACIFormat format, std::vector<std::vector<uint8_t>> &mipmaps, uint32_t width, uint32_t height);

	};

}

#endif
