#include "Packet.h"
#include "SocketPlatform.h"

#include <libunicode/utf8.h>

#include <cstring>

namespace libnet
{

	Packet::Packet(uint16_t id)
	: position(0)
	, id(id)
	, headerSent(false)
	{
		this->data.reserve(256);
	}

	uint16_t Packet::b_ntohs(uint16_t value)
	{
		return ((value & 0xff00) >> 8) | ((value & 0xff) << 8);
	}

	uint32_t Packet::b_ntohl(uint32_t value)
	{
		return ((value >> 24) & 0xff) | ((value >> 8) & 0xff00) | ((value & 0xff00) << 8) | ((value & 0xff) << 24);
	}

	uint64_t Packet::b_ntohll(uint64_t value)
	{
		return ((value >> 56) & 0xff) | ((value >> 40) & 0xff00) | ((value >> 24) & 0xff0000) | ((value >> 8) & 0xff000000 | ((value & 0xff000000) << 8) | ((value & 0xff0000) << 24) | ((value & 0xff00) << 40) | ((value & 0xff) << 56));
	}

	void Packet::writeBytes(const void *src, size_t len)
	{
		size_t pos = this->data.size();
		this->data.resize(this->data.size() + len);
		std::memcpy(this->data.data() + pos, src, len);
	}

	void Packet::writeBool(bool value)
	{
		uint8_t val = value ? 1 : 0;
		writeBytes(&val, 1);
	}

	void Packet::writeInt8(int8_t value)
	{
		writeBytes(&value, 1);
	}

	void Packet::writeUInt8(uint8_t value)
	{
		writeBytes(&value, 1);
	}

	void Packet::writeInt16(int16_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint16_t tmp = b_ntohs(*reinterpret_cast<uint16_t*>(&value));
		writeBytes(&tmp, 2);
#else
		writeBytes(&value, 2);
#endif
	}

	void Packet::writeUInt16(uint16_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint16_t tmp = b_ntohs(*reinterpret_cast<uint16_t*>(&value));
		writeBytes(&tmp, 2);
#else
		writeBytes(&value, 2);
#endif
	}

	void Packet::writeInt32(int32_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t tmp = b_ntohl(*reinterpret_cast<uint32_t*>(&value));
		writeBytes(&tmp, 4);
#else
		writeBytes(&value, 4);
#endif
	}

	void Packet::writeUInt32(uint32_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t tmp = b_ntohl(*reinterpret_cast<uint32_t*>(&value));
		writeBytes(&tmp, 4);
#else
		writeBytes(&value, 4);
#endif
	}

	void Packet::writeInt64(int64_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t tmp = b_ntohll(*reinterpret_cast<uint64_t*>(&value));
		writeBytes(&tmp, 8);
#else
		writeBytes(&value, 8);
#endif
	}

	void Packet::writeUInt64(uint64_t value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t tmp = b_ntohll(*reinterpret_cast<uint64_t*>(&value));
		writeBytes(&tmp, 8);
#else
		writeBytes(&value, 8);
#endif
	}

	void Packet::writeFloat(float value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t tmp = b_ntohl(*reinterpret_cast<uint32_t*>(&value));
		writeBytes(&tmp, 4);
#else
		writeBytes(&value, 4);
#endif
	}

	void Packet::writeDouble(double value)
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t tmp = b_ntohll(*reinterpret_cast<uint64_t*>(&value));
		writeBytes(&tmp, 8);
#else
		writeBytes(&value, 8);
#endif
	}

	void Packet::writeString(const std::string &value)
	{
		writeUInt16(value.length());
		writeBytes(value.data(), value.length());
	}

	void Packet::readBytes(void *dst, size_t len)
	{
		if (this->position + len > this->data.size())
			throw std::underflow_error("Packet buffer underflow (position = " + std::to_string(this->position) + ", size = " + std::to_string(this->data.size()) + ", len = " + std::to_string(len) + ")");
		std::memcpy(dst, this->data.data() + this->position, len);
		this->position += len;
	}

	bool Packet::readBool()
	{
		uint8_t value;
		readBytes(&value, 1);
		return value != 0;
	}

	int8_t Packet::readInt8()
	{
		int8_t value;
		readBytes(&value, 1);
		return value;
	}

	uint8_t Packet::readUInt8()
	{
		uint8_t value;
		readBytes(&value, 1);
		return value;
	}

	int16_t Packet::readInt16()
	{
		int16_t value;
		readBytes(&value, 2);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint16_t tmp = b_ntohs(*reinterpret_cast<uint16_t*>(&value));
		return *reinterpret_cast<uint16_t*>(&tmp);
#else
		return value;
#endif
	}

	uint16_t Packet::readUInt16()
	{
		uint16_t value;
		readBytes(&value, 2);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint16_t tmp = b_ntohs(*reinterpret_cast<uint16_t*>(&value));
		return *reinterpret_cast<uint16_t*>(&tmp);
#else
		return value;
#endif
	}

	int32_t Packet::readInt32()
	{
		int32_t value;
		readBytes(&value, 4);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t tmp = b_ntohl(*reinterpret_cast<uint32_t*>(&value));
		return *reinterpret_cast<uint32_t*>(&tmp);
#else
		return value;
#endif
	}

	uint32_t Packet::readUInt32()
	{
		uint32_t value;
		readBytes(&value, 4);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint32_t tmp = b_ntohl(*reinterpret_cast<uint32_t*>(&value));
		return *reinterpret_cast<uint32_t*>(&tmp);
#else
		return value;
#endif
	}

	int64_t Packet::readInt64()
	{
		int64_t value;
		readBytes(&value, 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t tmp = b_ntohll(*reinterpret_cast<uint64_t*>(&value));
		return *reinterpret_cast<uint64_t*>(&tmp);
#else
		return value;
#endif
	}

	uint64_t Packet::readUInt64()
	{
		uint64_t value;
		readBytes(&value, 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t tmp = b_ntohll(*reinterpret_cast<uint64_t*>(&value));
		return *reinterpret_cast<uint64_t*>(&tmp);
#else
		return value;
#endif
	}

	float Packet::readFloat()
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

	double Packet::readDouble()
	{
		double value;
		readBytes(&value, 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
		uint64_t tmp = b_ntohll(*reinterpret_cast<uint64_t*>(&value));
		return *reinterpret_cast<float*>(&tmp);
#else
		return value;
#endif
	}

	std::string Packet::readString()
	{
		std::string value;
		int16_t length = readUInt16();
		value.resize(length);
		readBytes(const_cast<char*>(value.c_str()), length);
		{
			std::string tmp;
			utf8::replace_invalid(value.begin(), value.end(), back_inserter(tmp));
			value = tmp;
		}
		return value;
	}

	void Packet::resize(uint64_t len)
	{
		this->data.resize(len);
	}

}
