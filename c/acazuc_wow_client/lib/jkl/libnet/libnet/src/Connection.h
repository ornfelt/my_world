#ifndef LIBNET_CONNECTION_H
# define LIBNET_CONNECTION_H

# include "Buffer.h"
# include "Socket.h"
# include "Packet.h"
# include <cstdint>
# include <string>
# include <queue>

namespace libnet
{

	class Connection
	{

		private:
			std::queue<Packet*> packets;
			Packet *currentPacket;
			Socket &socket;
			Buffer rBuffer;
			Buffer wBuffer;
			uint32_t packetStart;
			bool useShortPacketLength;
			bool checkWritePacket(Packet &packet);

		public:
			Connection(Socket &socket);
			~Connection();
			void startPacket(const uint16_t packet);
			void endPacket();
			bool hasCompletePacket(uint32_t &packetSize);
			bool initCrypt(const void *key, size_t keylen);
			void disableCrypt();
			int32_t recv();
			int32_t send();
			void writeBytes(const void *data, size_t len);
			void readBytes(void *data, size_t len);
			void writeBool(const bool value);
			bool readBool();
			void writeInt8(const int8_t value);
			int8_t readInt8();
			void writeUInt8(const uint8_t value);
			uint8_t readUInt8();
			void writeInt16(const int16_t value);
			int16_t readInt16();
			void writeUInt16(const uint16_t value);
			uint16_t readUInt16();
			void writeInt32(const int32_t value);
			int32_t readInt32();
			void writeUInt32(const uint32_t value);
			uint32_t readUInt32();
			void writeInt64(const int64_t value);
			int64_t readInt64();
			void writeUInt64(const uint64_t value);
			uint64_t readUInt64();
			void writeFloat(const float value);
			float readFloat();
			void writeDouble(const double value);
			double readDouble();
			void writeString(const std::string &value);
			std::string readString();
			inline std::queue<Packet*> &getPackets() {return this->packets;};
			inline bool setBlocking(bool blocking) {return this->socket.setBlocking(blocking);};
			inline bool setNagle(bool nagle) {return this->socket.setNagle(nagle);};
			inline void setUseShortPacketLength(bool use) {this->useShortPacketLength = use;};
			inline Socket &getSocket() {return this->socket;};
			inline Buffer &getRBuffer() {return this->rBuffer;};
			inline Buffer &getWBuffer() {return this->wBuffer;};

	};

}

#endif
