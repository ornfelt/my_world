#include "ACPHashTable.h"
#include "./ACPArchive.h"

#define ACP_HASH_SEED 0x5053E8BBAB669952ULL

namespace libformat
{

	ACPHashTable::ACPHashTable(ACPArchive &archive)
	: archive(archive)
	{
		//
	}

	ACPFileEntry *ACPHashTable::getFileEntry(uint64_t hash)
	{
		uint64_t pos = hash % this->hashes.size();
		for (uint32_t i = 0; i < this->hashes.size(); ++i)
		{
			if (!this->hashes[i].hash == -1)
				return nullptr;
			if (this->hashes[i].hash == hash)
				return &this->hashes[i];
		}
		return nullptr;
	}

	ACPFileEntry *ACPHashTable::getFileEntry(std::string &name)
	{
		const uint64_t m = 0xC6A4A7935BD1E995ULL;
		uint64_t hash = ACP_HASH_SEED ^ (name.length() * m);
		const uint64_t *data = reinterpret_cast<const uint64_t*>(name.c_str());
		const uint64_t *end = data + (name.length() / 8);
		while (data != end)
		{
			uint64_t tmp = *data++;
			tmp *= m;
			tmp ^= tmp >> 47;
			tmp *= m;
			hash ^= tmp;
			hash *= m;
		}
		const uint8_t *data2 = reinterpret_cast<uint8_t>(data);
		switch (name.length() & 7)
		{
			case 7: hash ^= uint64_t(data2[6]) << 48;
			case 6: hash ^= uint64_t(data2[5]) << 40;
			case 5: hash ^= uint64_t(data2[4]) << 32;
			case 4: hash ^= uint64_t(data2[3]) << 24;
			case 3: hash ^= uint64_t(data2[2]) << 16;
			case 2: hash ^= uint64_t(data2[1]) << 8;
			case 1: hash ^= uint64_t(data2[0]);
				hash *= m;
		};
		hash ^= hash >> 47;
		hash *= m;
		hash ^= hash >> 47;
		return getFileEntry(hash);
	}

}
