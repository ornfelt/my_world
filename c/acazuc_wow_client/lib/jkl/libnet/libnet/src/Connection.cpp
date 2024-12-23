#include "Connection.h"

namespace libnet
{

	Connection::Connection(Socket &socket)
	: currentPacket(NULL)
	, socket(socket)
	, rBuffer(1024)
	, wBuffer(1024)
	, useShortPacketLength(false)
	{
		rBuffer.flip();
	}

	Connection::~Connection()
	{
		delete (currentPacket);
		while (this->packets.size())
		{
			delete (this->packets.front());
			this->packets.pop();
		}
	}

	void Connection::startPacket(const uint16_t packet)
	{
		this->currentPacket = new Packet(packet);
	}

	void Connection::endPacket()
	{
		if (!this->currentPacket)
			return;
		if (!this->packets.size())
		{
			if (checkWritePacket(*this->currentPacket))
				delete (this->currentPacket);
			else
				this->packets.push(this->currentPacket);
		}
		else
		{
			this->packets.push(this->currentPacket);
		}
		this->currentPacket = NULL;
	}

	bool Connection::hasCompletePacket(uint32_t &packetSize)
	{
		if (this->useShortPacketLength)
		{
			if (this->rBuffer.getRemaining() < 1)
				return false;
			uint8_t first = this->rBuffer.readUInt8();
			if (!(first & 0x80))
			{
				packetSize = first;
				if (this->rBuffer.getRemaining() < packetSize)
				{
					this->rBuffer.setPosition(this->rBuffer.getPosition() - 1);
					return false;
				}
				return true;
			}
			if ((first & 0xC0) == 0x80)
			{
				if (this->rBuffer.getRemaining() < 1)
				{
					this->rBuffer.setPosition(this->rBuffer.getPosition() - 1);
					return false;
				}
				packetSize = static_cast<uint32_t>(first & 0x3F) << 8;
				packetSize |= this->rBuffer.readUInt8();
				if (this->rBuffer.getRemaining() < packetSize)
				{
					this->rBuffer.setPosition(this->rBuffer.getPosition() - 2);
					return false;
				}
				return true;
			}
			if ((first & 0xE) == 0xC0)
			{
				if (this->rBuffer.getRemaining() < 2)
				{
					this->rBuffer.setPosition(this->rBuffer.getPosition() - 1);
					return false;
				}
				packetSize = static_cast<uint32_t>(first & 0x1F) << 16;
				packetSize |= static_cast<uint32_t>(this->rBuffer.readUInt8()) << 8;
				packetSize |= this->rBuffer.readUInt8();
				if (this->rBuffer.getRemaining() < packetSize)
				{
					this->rBuffer.setPosition(this->rBuffer.getPosition() - 3);
					return false;
				}
				return true;
			}
			if (this->rBuffer.getRemaining() < 3)
			{
				this->rBuffer.setPosition(this->rBuffer.getPosition() - 1);
				return false;
			}
			packetSize = static_cast<uint32_t>(first & 0xF) << 24;
			packetSize |= static_cast<uint32_t>(this->rBuffer.readUInt8()) << 16;
			packetSize |= static_cast<uint32_t>(this->rBuffer.readUInt8()) << 8;
			packetSize |= this->rBuffer.readUInt8();
			if (this->rBuffer.getRemaining() < packetSize)
			{
				this->rBuffer.setPosition(this->rBuffer.getPosition() - 4);
				return false;
			}
			return true;
		}
		else
		{
			if (this->rBuffer.getRemaining() < 6)
				return false;
			packetSize = this->rBuffer.readUInt32();
			if (this->rBuffer.getRemaining() < packetSize)
			{
				this->rBuffer.setPosition(this->rBuffer.getPosition() - 4);
				return false;
			}
			return true;
		}
	}

	bool Connection::checkWritePacket(Packet &packet)
	{
		if (!this->wBuffer.getRemaining())
			return false;
		if (!packet.isHeaderSent())
		{
			if (this->wBuffer.getRemaining() < 6)
				return false;
			if (this->useShortPacketLength)
			{
				uint32_t len = packet.getData().size() + 2;
				if (len <= 0x7F)
				{
					this->wBuffer.writeUInt8(len);
				}
				else if (len <= 0x3FFF)
				{
					this->wBuffer.writeUInt8(((len >> 8) & 0x3F) | 0x80);
					this->wBuffer.writeUInt8(len & 0xFF);
				}
				else if (len <= 0x1FFFFF)
				{
					this->wBuffer.writeUInt8(((len >> 16) & 0x1F) | 0xC0);
					this->wBuffer.writeUInt8((len >> 8) & 0xFF);
					this->wBuffer.writeUInt8(len & 0xFF);
				}
				else
				{
					this->wBuffer.writeUInt8(((len >> 24) & 0xF) | 0xE0);
					this->wBuffer.writeUInt8((len >> 16) & 0xFF);
					this->wBuffer.writeUInt8((len >> 8) & 0xFF);
					this->wBuffer.writeUInt8(len & 0xFF);
				}
			}
			else
			{
				this->wBuffer.writeUInt32(packet.getData().size() + 2);
			}
			this->wBuffer.writeUInt16(packet.getId());
			packet.setHeaderSent(true);
		}
		if (this->wBuffer.getRemaining() < packet.getRemaining())
		{
			int32_t remaining = this->wBuffer.getRemaining();
			this->wBuffer.writeBytes(packet.getData().data() + packet.getPosition(), remaining);
			packet.setPosition(packet.getPosition() + remaining);
			return false;
		}
		this->wBuffer.writeBytes(packet.getData().data() + packet.getPosition(), packet.getRemaining());
		packet.setPosition(packet.getData().size());
		return true;
	}

	bool Connection::initCrypt(const void *key, size_t keylen)
	{
		if (!this->rBuffer.initCrypt(key, keylen))
			return false;
		if (!this->wBuffer.initCrypt(key, keylen))
			return false;
		return true;
	}

	void Connection::disableCrypt()
	{
		this->rBuffer.disableCrypt();
		this->wBuffer.disableCrypt();
	}

	int32_t Connection::recv()
	{
		return this->socket.recv(this->rBuffer);
	}

	int32_t Connection::send()
	{
		while (this->packets.size())
		{
			Packet *packet = this->packets.front();
			if (!checkWritePacket(*packet))
				break;
			this->packets.pop();
			delete (packet);
		}
		return this->socket.send(this->wBuffer);
	}

	void Connection::writeBytes(const void *data, size_t len)
	{
		this->currentPacket->writeBytes(data, len);
	}

	void Connection::readBytes(void *data, size_t len)
	{
		this->rBuffer.readBytes(data, len);
	}

	void Connection::writeBool(const bool value)
	{
		this->currentPacket->writeBool(value);
	}

	bool Connection::readBool()
	{
		return this->rBuffer.readBool();
	}

	void Connection::writeInt8(const int8_t value)
	{
		this->currentPacket->writeInt8(value);
	}

	int8_t Connection::readInt8()
	{
		return this->rBuffer.readInt8();
	}

	void Connection::writeUInt8(const uint8_t value)
	{
		this->currentPacket->writeUInt8(value);
	}

	uint8_t Connection::readUInt8()
	{
		return this->rBuffer.readUInt8();
	}

	void Connection::writeInt16(const int16_t value)
	{
		this->currentPacket->writeInt16(value);
	}

	int16_t Connection::readInt16()
	{
		return this->rBuffer.readInt16();
	}

	void Connection::writeUInt16(const uint16_t value)
	{
		this->currentPacket->writeUInt16(value);
	}

	uint16_t Connection::readUInt16()
	{
		return this->rBuffer.readUInt16();
	}

	void Connection::writeInt32(const int32_t value)
	{
		this->currentPacket->writeInt32(value);
	}

	int32_t Connection::readInt32()
	{
		return this->rBuffer.readInt32();
	}

	void Connection::writeUInt32(const uint32_t value)
	{
		this->currentPacket->writeUInt32(value);
	}

	uint32_t Connection::readUInt32()
	{
		return this->rBuffer.readUInt32();
	}

	void Connection::writeInt64(const int64_t value)
	{
		this->currentPacket->writeInt64(value);
	}

	int64_t Connection::readInt64()
	{
		return this->rBuffer.readInt64();
	}

	void Connection::writeUInt64(const uint64_t value)
	{
		this->currentPacket->writeUInt64(value);
	}

	uint64_t Connection::readUInt64()
	{
		return this->rBuffer.readUInt64();
	}

	void Connection::writeFloat(const float value)
	{
		this->currentPacket->writeFloat(value);
	}

	float Connection::readFloat()
	{
		return this->rBuffer.readFloat();
	}

	void Connection::writeDouble(double value)
	{
		this->currentPacket->writeDouble(value);
	}

	double Connection::readDouble()
	{
		return this->rBuffer.readDouble();
	}

	void Connection::writeString(const std::string &value)
	{
		this->currentPacket->writeString(value);
	}

	std::string Connection::readString()
	{
		return this->rBuffer.readString();
	}

}
