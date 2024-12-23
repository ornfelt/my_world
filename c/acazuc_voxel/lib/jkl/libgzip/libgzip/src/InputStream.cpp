#include "InputStream.h"
#include <cstring>

#define CHUNK 16384

namespace gz
{

	InputStream::InputStream()
	: bufferOff(0)
	, bufferLen(0)
	{
		this->buffer.resize(CHUNK);
	}

	InputStream::~InputStream()
	{
		//Empty
	}

	ssize_t InputStream::read(void *data, size_t len)
	{
		size_t written = 0;
		do
		{
			if (this->bufferLen > 0 && this->bufferOff > 0)
			{
				std::memmove(this->buffer.data(), this->buffer.data() + this->bufferOff, this->bufferLen);
				this->bufferOff = 0;
			}
			ssize_t readed = readBytes(this->buffer.data() + this->bufferLen, this->buffer.size() - this->bufferLen);
			if (readed <= 0)
			{
				if (!eof())
					return -1;
				if (this->bufferLen == 0)
					break;
			}
			this->bufferLen += readed;
			this->stream.avail_in = this->bufferLen;
			if (this->stream.avail_in == 0)
				break;
			this->stream.next_in = this->buffer.data();
			this->stream.avail_out = len - written;
			this->stream.next_out = (Bytef*)data + written;
			int ret = inflate(&this->stream, Z_NO_FLUSH);
			if (ret != Z_OK && ret != Z_FINISH && ret != Z_STREAM_END)
				return -1;
			this->bufferOff = this->bufferLen - this->stream.avail_in;
			written = len - this->stream.avail_out;
			this->bufferLen = this->stream.avail_in;
			if (ret == Z_STREAM_END)
				break;
		} while (written < len);
		return written;
	}

	bool InputStream::readInt8(int8_t *val)
	{
		return read(val, 1) == 1;
	}

	bool InputStream::readUInt8(uint8_t *val)
	{
		return read(val, 1) == 1;
	}

	bool InputStream::readInt16(int16_t *val)
	{
		return read(val, 2) == 2;
	}

	bool InputStream::readUInt16(uint16_t *val)
	{
		return read(val, 2) == 2;
	}

	bool InputStream::readInt32(int32_t *val)
	{
		return read(val, 4) == 4;
	}

	bool InputStream::readUInt32(uint32_t *val)
	{
		return read(val, 4) == 4;
	}

	bool InputStream::readInt64(int64_t *val)
	{
		return read(val, 8) == 8;
	}

	bool InputStream::readUInt64(uint64_t *val)
	{
		return read(val, 8) == 8;
	}

}
