#ifndef ACP_FILE_H
# define ACP_FILE_H

namespace libformat
{

	struct ACPFileEntry
	{
		uint64_t offset;
		uint64_t dataSize;
		uint64_t fileSize;
		uint32_t flags;
	};

	struct ACPSector
	{
		uint64_t offset;
		uint64_t size;
	};

	enum ACPFileSeekMode
	{
		ACP_SEEK_SET,
		ACP_SEEK_CUR,
		ACP_SEEK_END
	};

	class ACPFile
	{

	private:
		ACPArchive &archive;
		std::vector<ACPSector> sectors;
		std::vector<uint8_t> sectorDatas;
		uint64_t currentSector;
		uint64_t sectorPos;
		uint32_t mode;
		uint64_t pos;

	public:
		ACPFile(ACPArchive &archive, ACPFileEntry &entry, uint32_t mode);
		~ACPFile();
		int64_t read(void *data, uint64_t len);
		int64_t write(void *data, uint64_t len);
		int64_t seek(uint64_t pos, enum ACPFileSeekMode mode);
		int64_t truncate(uint64_t len);
		int64_t tell();

	};

}

#endif
