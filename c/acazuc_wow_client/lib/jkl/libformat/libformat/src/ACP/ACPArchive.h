#ifndef ACP_ARCHIVE_H
# define ACP_ARCHIVE_H

# define ACP_RDONLY 0x1
# define ACP_WRONLY 0x2
# define ACP_RDWR 0x3
# define ACP_CREAT 0x4
# define ACP_TRUNC 0x8

# include "./ACPFile.h"
# include <fstream>
# include <cstdint>
# include <string>

namespace libformat
{

	struct ACPHeader
	{
		uint32_t magic;
		uint32_t version;
		uint64_t hashTableOffset;
		uint64_t hashTableSize;
	};

	class ACPArchive
	{

	private:
		ACPHashTable hashTable;
		std::fstream stream;
		ACPHeader header;

	public:
		ACPArchive(std::string filename);
		ACPFile *open(std::string filename, uint32_t mode);
		bool remove(std::string filename);

	};

}

#endif
