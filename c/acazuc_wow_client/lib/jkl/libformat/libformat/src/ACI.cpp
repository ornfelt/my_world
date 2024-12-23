#include "ACI.h"
#include <libgzip/MemoryInputStream.h>
#include <libgzip/MemoryOutputStream.h>
#include <cstring>
#include <cstdio>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <arpa/inet.h>
#endif

#define MAGIC_NUMBER (('A' << 24) | ('C' << 16) | ('I' << 8) | 0x97)

namespace libformat
{

	bool ACI::read(std::string filename, enum ACIFormat &format, std::vector<std::vector<uint8_t>> &mipmaps, uint32_t &width, uint32_t &height)
	{
		mipmaps.clear();
		FILE *file;
		struct ACIHeader header;
		if (!(file = std::fopen(filename.c_str(), "rb")))
			goto err1;
		if (std::fread(&header, sizeof(header), 1, file) != 1)
			goto err2;
		header.magic = ntohl(header.magic);
		header.width = ntohl(header.width);
		header.height = ntohl(header.height);
		width = header.width;
		height = header.height;
		if (header.magic != MAGIC_NUMBER)
			goto err2;
		if (header.version != 1)
			goto err2;
		if (header.format != static_cast<uint8_t>(ACI_BGRA8)
				&& header.format != static_cast<uint8_t>(ACI_RG8)
				&& header.format != static_cast<uint8_t>(ACI_R8)
				&& header.format != static_cast<uint8_t>(ACI_BC1)
				&& header.format != static_cast<uint8_t>(ACI_BC2)
				&& header.format != static_cast<uint8_t>(ACI_BC3)
				&& header.format != static_cast<uint8_t>(ACI_BC4)
				&& header.format != static_cast<uint8_t>(ACI_BC5)
				&& header.format != static_cast<uint8_t>(ACI_BC6)
				&& header.format != static_cast<uint8_t>(ACI_BC7))
			goto err2;
		format = static_cast<enum ACIFormat>(header.format);
		mipmaps.resize(header.mipmaps);
		{
		for (uint32_t i = 0; i < header.mipmaps; ++i)
		{
			uint32_t dataSize;
			uint32_t fileSize;
			std::vector<uint8_t> fileData;
			if (std::fread(&dataSize, sizeof(dataSize), 1, file) != 1)
				goto err2;
			dataSize = ntohl(dataSize);
			mipmaps[i].resize(dataSize);
			if (std::fread(&fileSize, sizeof(fileSize), 1, file) != 1)
				goto err2;
			fileSize = ntohl(fileSize);
			fileData.resize(fileSize);
			if (std::fread(fileData.data(), fileData.size(), 1, file) != 1)
				goto err2;
			switch (header.compression)
			{
				case ACI_COMPRESSION_NONE:
				{
					fileData.swap(mipmaps[i]);
					break;
				}
				case ACI_COMPRESSION_ZLIB:
				{
					try
					{
						gz::MemoryInputStream stream(fileData.data(), fileData.size());
						if (stream.read(mipmaps[i].data(), mipmaps[i].size()) != mipmaps[i].size())
							goto err2;
					}
					catch (std::exception &e)
					{
						goto err2;
					}
					break;
				}
				default:
					goto err2;
			}
		}
		}
		std::fclose(file);
		return true;
	err2:
		std::fclose(file);
	err1:
		return false;
	}

	bool ACI::write(std::string filename, enum ACICompressionAlgorithm compression, enum ACIFormat format, std::vector<std::vector<uint8_t>> &mipmaps, uint32_t width, uint32_t height)
	{
		if (mipmaps.size() > 0xff)
			return false;
		FILE *file;
		struct ACIHeader header;
		header.magic = htonl(MAGIC_NUMBER);
		header.version = 0x1;
		header.mipmaps = mipmaps.size();
		header.format = static_cast<uint8_t>(format);
		header.compression = static_cast<uint8_t>(compression);
		header.width = htonl(width);
		header.height = htonl(height);
		if (!(file = std::fopen(filename.c_str(), "wb")))
			goto err1;
		if (std::fwrite(&header, sizeof(header), 1, file) != 1)
			goto err2;
		for (uint32_t i = 0; i < mipmaps.size(); ++i)
		{
			switch (compression)
			{
				case ACI_COMPRESSION_NONE:
				{
					uint32_t dataSize = htonl(mipmaps[i].size());
					if (std::fwrite(&dataSize, sizeof(dataSize), 1, file) != 1)
						goto err2;
					uint32_t fileSize = dataSize;
					if (std::fwrite(&fileSize, sizeof(fileSize), 1, file) != 1)
						goto err2;
					if (std::fwrite(mipmaps[i].data(), mipmaps[i].size(), 1, file) != 1)
						goto err2;
					break;
				}
				case ACI_COMPRESSION_ZLIB:
				{
					try
					{
						gz::MemoryOutputStream stream(9);
						if (stream.write(mipmaps[i].data(), mipmaps[i].size()) != mipmaps[i].size())
							goto err2;
						stream.flush();
						std::vector<uint8_t> &datas = stream.getData();
						uint32_t dataSize = htonl(mipmaps[i].size());
						if (std::fwrite(&dataSize, sizeof(dataSize), 1, file) != 1)
							goto err2;
						uint32_t fileSize = htonl(datas.size());
						if (std::fwrite(&fileSize, sizeof(fileSize), 1, file) != 1)
							goto err2;
						if (std::fwrite(datas.data(), datas.size(), 1, file) != 1)
							goto err2;
					}
					catch (std::exception &e)
					{
						goto err2;
					}
				}
			}
		}
		std::fclose(file);
		return true;
	err2:
		std::fclose(file);
	err1:
		return false;
	}

}
