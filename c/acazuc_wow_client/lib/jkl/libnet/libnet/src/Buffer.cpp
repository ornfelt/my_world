#include "Buffer.h"
#include "SocketPlatform.h"

#include <libunicode/utf8.h>

#include <stdexcept>
#include <cstring>

namespace libnet
{

	Buffer::Buffer(uint32_t capacity)
	: position(0)
	, limit(capacity)
	, crypted(false)
	{
		this->data.resize(capacity);
	}

	uint16_t Buffer::b_ntohs(uint16_t value)
	{
		return ((value & 0xff00) >> 8) | ((value & 0xff) << 8);
	}

	uint32_t Buffer::b_ntohl(uint32_t value)
	{
		return ((value >> 24) & 0xff) | ((value >> 8) & 0xff00) | ((value & 0xff00) << 8) | ((value & 0xff) << 24);
	}

	uint64_t Buffer::b_ntohll(uint64_t value)
	{
		return ((value >> 56) & 0xff) | ((value >> 40) & 0xff00) | ((value >> 24) & 0xff0000) | ((value >> 8) & 0xff000000 | ((value & 0xff000000) << 8) | ((value & 0xff0000) << 24) | ((value & 0xff00) << 40) | ((value & 0xff) << 56));
	}

	void Buffer::crypt(uint32_t position, uint32_t length)
	{
		for (uint32_t i = 0; i < length; ++i)
		{
			++this->cryptPos1;
			this->cryptPos2 += this->cryptBox[this->cryptPos1];
			std::swap(this->cryptBox[this->cryptPos1], this->cryptBox[this->cryptPos2]);
			uint8_t k = this->cryptBox[this->cryptPos1] + this->cryptBox[this->cryptPos2];
			this->data[position + i] ^= this->cryptBox[k];
		}
	}

	bool Buffer::initCrypt(const void *key, size_t keylen)
	{
		if (keylen == 0)
			return false;
		this->cryptBox.resize(256);
		this->crypted = true;
		this->cryptPos1 = 0;
		this->cryptPos2 = 0;
		for (uint16_t i = 0; i < 256; ++i)
			this->cryptBox[i] = uint8_t(i);
		uint8_t j = 0;
		for (uint16_t i = 0; i < 256; ++i)
		{
			j += this->cryptBox[i] + (reinterpret_cast<const uint8_t*>(key))[i % keylen];
			std::swap(this->cryptBox[i], this->cryptBox[j]);
		}
		//Little bit better security (pre-calc some values)
		uint64_t max = 4096 + this->cryptBox[0] + this->cryptBox[1];
		for (uint16_t i = 0; i < max; ++i)
		{
			++this->cryptPos1;
			this->cryptPos2 += this->cryptBox[this->cryptPos1];
			std::swap(this->cryptBox[this->cryptPos1], this->cryptBox[this->cryptPos2]);
		}
		return true;
	}

	void Buffer::disableCrypt()
	{
		this->crypted = false;
	}

	void Buffer::writeBytes(const void *src, uint32_t len)
	{
		if (this->position + len > this->limit)
			throw std::overflow_error("position = " + std::to_string(this->position) + "; limit = " + std::to_string(this->limit) + "; len = " + std::to_string(len));
		std::memmove(&this->data[this->position], src, len);
		if (this->crypted)
			crypt(this->position, len);
		setPosition(this->position + len);
	}

	void Buffer::writeBool(bool value)
	{
		writeUInt8(value ? 1 : 0);
	}

	void Buffer::writeInt8(int8_t value)
	{
		writeBytes(&value, 1);
	}

	void Buffer::writeUInt8(uint8_t value)
	{
		writeBytes(&value, 1);
	}

	void Buffer::writeInt16(int16_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint16_t val = b_ntohs(*reinterpret_cast<uint16_t*>(&value));
		writeBytes(&val, 2);
#else
		writeBytes(&value, 2);
#endif
	}

	void Buffer::writeUInt16(uint16_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint16_t val = b_ntohs(value);
		writeBytes(&val, 2);
#else
		writeBytes(&value, 2);
#endif
	}

	void Buffer::writeInt32(int32_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t val = b_ntohl(*reinterpret_cast<uint32_t*>(&value));
		writeBytes(&val, 4);
#else
		writeBytes(&value, 4);
#endif
	}

	void Buffer::writeUInt32(uint32_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t val = b_ntohl(value);
		writeBytes(&val, 4);
#else
		writeBytes(&value, 4);
#endif
	}

	void Buffer::writeInt64(int64_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t val = b_ntohll(*reinterpret_cast<uint64_t*>(&value));
		writeBytes(&val, 8);
#else
		writeBytes(&value, 8);
#endif
	}

	void Buffer::writeUInt64(uint64_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t val = b_ntohll(value);
		writeBytes(&val, 8);
#else
		writeBytes(&value, 8);
#endif
	}

	void Buffer::writeFloat(float value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t val = b_ntohl(*reinterpret_cast<uint32_t*>(&value));
		writeBytes(&val, 4);
#else
		writeBytes(&value, 4);
#endif
	}

	void Buffer::writeDouble(double value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t val = b_ntohll(*reinterpret_cast<uint64_t*>(&value));
		writeBytes(&val, 8);
#else
		writeBytes(&value, 8);
#endif
	}

	void Buffer::writeString(const std::string &value)
	{
		writeUInt16(value.length());
		writeBytes(value.c_str(), value.length());
	}

	void Buffer::readBytes(void *dst, uint32_t len)
	{
		if (this->position + len > this->limit)
			throw std::underflow_error("position = " + std::to_string(this->position) + "; limit = " + std::to_string(this->limit) + "; len = " + std::to_string(len));
		std::memmove(dst, &this->data[this->position], len);
		setPosition(this->position + len);
	}

	bool Buffer::readBool()
	{
		return readUInt8() != 0;
	}

	int8_t Buffer::readInt8()
	{
		int8_t value;
		readBytes(&value, 1);
		return value;
	}

	uint8_t Buffer::readUInt8()
	{
		uint8_t value;
		readBytes(&value, 1);
		return value;
	}

	int16_t Buffer::readInt16()
	{
		int16_t value;
		readBytes(&value, 2);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint16_t tmp = b_ntohs(*reinterpret_cast<uint16_t*>(&value));
		return *reinterpret_cast<int16_t*>(&tmp);
#else
		return value;
#endif
	}

	uint16_t Buffer::readUInt16()
	{
		uint16_t value;
		readBytes(&value, 2);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		return b_ntohs(value);
#else
		return value;
#endif
	}

	int32_t Buffer::readInt32()
	{
		int32_t value;
		readBytes(&value, 4);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t tmp = b_ntohl(*reinterpret_cast<uint32_t*>(&value));
		return *reinterpret_cast<int32_t*>(&tmp);
#else
		return value;
#endif
	}

	uint32_t Buffer::readUInt32()
	{
		uint32_t value;
		readBytes(&value, 4);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		return b_ntohl(value);
#else
		return value;
#endif
	}

	int64_t Buffer::readInt64()
	{
		int64_t value;
		readBytes(&value, 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t tmp = b_ntohll(*reinterpret_cast<uint64_t*>(&value));
		return *reinterpret_cast<int64_t*>(&tmp);
#else
		return value;
#endif
	}

	uint64_t Buffer::readUInt64()
	{
		uint64_t value;
		readBytes(&value, 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		return b_ntohll(value);
#else
		return value;
#endif
	}

	float Buffer::readFloat()
	{
		float value;
		readBytes(&value, 4);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t tmp = b_ntohl(*reinterpret_cast<uint32_t*>(&value));
		return *reinterpret_cast<float*>(&tmp);
#else
		return value;
#endif
	}

	double Buffer::readDouble()
	{
		double value;
		readBytes(&value, 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t tmp = b_ntohl(*reinterpret_cast<uint64_t*>(&value));
		return *reinterpret_cast<double*>(&tmp);
#else
		return value;
#endif
	}

	std::string Buffer::readString()
	{
		std::string str;
		int16_t length = readUInt16();
		str.resize(length);
		readBytes(const_cast<char*>(str.c_str()), length);
		{
			std::string tmp;
			utf8::replace_invalid(str.begin(), str.end(), back_inserter(tmp));
			str = tmp;
		}
		return str;
	}

	void Buffer::resize(uint32_t len)
	{
		if (len == 0)
			len = 1;
		std::vector<uint8_t> newData(len);
		std::copy(this->data.begin(), this->data.end(), newData.begin());
		this->data.swap(newData);
		if (this->limit >= len || this->limit == this->data.size())
		{
			setLimit(len);
			if (this->position > this->limit)
				setPosition(this->limit);
		}
	}

	void Buffer::setPosition(uint32_t position)
	{
		this->position = position;
	}

	void Buffer::setLimit(uint32_t limit)
	{
		this->limit = limit;
	}

	void Buffer::clear()
	{
		setLimit(this->data.size());
		setPosition(0);
	}

	void Buffer::flip()
	{
		setLimit(this->position);
		setPosition(0);
	}

}
