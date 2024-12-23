#ifndef ACP_HASH_TABLE_H
# define ACP_HASH_TABLE_H

namespace libformat
{

	struct ACPHashEntry
	{
		uint64_t hash;
		ACPFileEntry fileEntry;
	};

	class ACPArchive;

	class ACPHashTable
	{

	private:
		std::vector<ACPHashEntry> hashes;
		ACPArchive &parent;

	public:
		ACPHashTable(ACPArchive &archive);
		ACPFileEntry *getFileEntry(uint64_t hash);
		ACPFileEntry *getFileEntry(std::string &name);

	};

}

#endif
