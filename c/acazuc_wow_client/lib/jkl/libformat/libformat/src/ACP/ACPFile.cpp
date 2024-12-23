#include "ACPFile.h"

namespace libformat
{

	ACPFile::ACPFile(ACPArchive &archive, ACPFileEntry &entry, uint32_t mode)
	: currentSector(0)
	, sectorPos(0)
	, mode(mode)
	, pos(0)
	{
		this->sectorDatas.reserve(4096);
	}

	ACPFile::~ACPFile()
	{
		//
	}

	int64_t ACPFile::read(void *data, uint64_t len)
	{
		if (!len)
			return 0;
		uint64_t readed = 0;
		while (readed < len)
		{
			if (this->currentSector >= this->sectors.size())
				return readed;
			ACPSector &currentSector = this->sectors[this->currentSector];
		}
	}

	int64_t ACPFile::write(void *data, uint64_t len)
	{
		if (!len)
			return 0;
	}

	int64_t seek(uint64_t pos, enum ACPFileSeekMode mode)
	{
		switch (mode)
		{
			case ACP_SEEK_SET:
				break;
			case ACP_SEEK_CUR:
				break;
			case ACP_SEEK_END:
				break;
			default:
				return -1;
		}
	}

	int64_t truncate(uint64_t len)
	{
		//
	}

	int64_t tell()
	{
		return this->pos;
	}

}
