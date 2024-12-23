#ifndef LIBNET_PACKET_H
# define LIBNET_PACKET_H

# include <cstdint>
# include <vector>
# include <string>

namespace libnet
{

	class Packet
	{

	private:
		std::vector<uint8_t> data;
		uint32_t position;
		uint16_t id;
		bool headerSent;
		uint16_t b_ntohs(uint16_t value);
		uint32_t b_ntohl(uint32_t value);
		uint64_t b_ntohll(uint64_t value);

	public:
		Packet(const uint16_t id);
		void writeBytes(const void *src, size_t len);
		void writeBool(bool value);
		void writeInt8(int8_t value);
		void writeUInt8(uint8_t value);
		void writeInt16(int16_t value);
		void writeUInt16(uint16_t value);
		void writeInt32(int32_t value);
		void writeUInt32(uint32_t value);
		void writeInt64(int64_t value);
		void writeUInt64(uint64_t value);
		void writeFloat(float value);
		void writeDouble(double value);
		void writeString(const std::string &value);
		void readBytes(void *dst, size_t len);
		bool readBool();
		int8_t readInt8();
		uint8_t readUInt8();
		int16_t readInt16();
		uint16_t readUInt16();
		int32_t readInt32();
		uint32_t readUInt32();
		int64_t readInt64();
		uint64_t readUInt64();
		float readFloat();
		double readDouble();
		std::string readString();
		void resize(uint64_t len);
		inline std::vector<uint8_t> &getData() {return this->data;};
		inline void setPosition(uint32_t position) {this->position = position;};
		inline uint32_t getPosition() const {return this->position;};
		inline uint32_t getRemaining() const {return this->data.size() - this->position;};
		inline uint16_t getId() const {return this->id;};
		inline void setHeaderSent(bool headerSent) {this->headerSent = headerSent;};
		inline bool isHeaderSent() const {return this->headerSent;};

	};

}

#endif
