#include "MemoryOutputStream.h"
#include <cstring>

#define CHUNK 16384

namespace gz
{

	MemoryOutputStream::MemoryOutputStream(uint8_t level)
	{
		std::memset(&this->stream, 0, sizeof(this->stream));
		if (deflateInit(&this->stream, level) != Z_OK)
			throw std::exception();
	}

	MemoryOutputStream::~MemoryOutputStream()
	{
		deflateEnd(&this->stream);
	}

	ssize_t MemoryOutputStream::writeBytes(const void *data, size_t len)
	{
		this->data.resize(this->data.size() + len);
		std::memmove(this->data.data() + this->data.size() - len, data, len);
		return len;
	}

}
