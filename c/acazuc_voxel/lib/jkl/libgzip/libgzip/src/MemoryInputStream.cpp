#include "MemoryInputStream.h"
#include <cstring>

namespace gz
{

	MemoryInputStream::MemoryInputStream(void *data, size_t len)
	: len(len)
	, pos(0)
	, data(data)
	{
		std::memset(&this->stream, 0, sizeof(this->stream));
		if (inflateInit(&this->stream) != Z_OK)
			throw std::exception();
	}

	MemoryInputStream::~MemoryInputStream()
	{
		inflateEnd(&this->stream);
	}

	ssize_t MemoryInputStream::readBytes(void *data, size_t len)
	{
		ssize_t readed = std::min(this->len - this->pos, len);
		std::memmove(data, (uint8_t*)this->data + this->pos, readed);
		this->pos += readed;
		return readed;
	}

	bool MemoryInputStream::eof()
	{
		return this->pos == this->len;
	}

}
