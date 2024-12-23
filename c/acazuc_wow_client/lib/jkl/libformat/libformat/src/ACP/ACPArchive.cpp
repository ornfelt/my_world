#include "ACPFile.h"

namespace libformat
{

	ACPArchive::ACPArchive(std::string filename)
	: stream(filename, std::ios_base::in | std::io_base::out | std::ios_base::binary)
	{
		//
	}

	ACPFile *ACPArchive::open(std::string filename, uint32_t mode)
	{
		ACPFileEntry *entry = this->hashTable.getFileEntry(filename);
		if (entry)
		{
			ACPFile *file = new ACPFile(*this, entry, mode);
			if (mode & ACP_TRUNC)
				file->truncate();
		}
		if (!mode & ACP_CREAT)
			return nullptr;
		//TODO create file
		return nullptr
	}

	bool ACPArchive::removeFile(std::string filename)
	{
		//TODO
		(void)filename;
		return false;
	}

}
