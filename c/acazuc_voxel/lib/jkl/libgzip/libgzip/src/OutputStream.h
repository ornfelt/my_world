#ifndef GZ_OUTPUT_STREAM_H
# define GZ_OUTPUT_STREAM_H

# include <cstdint>
# include <cstdio>
# include <string>
# include <vector>
# include <zlib.h>

namespace gz
{

	class OutputStream
	{

	protected:
		std::vector<uint8_t> buffer;
		uint32_t bufferOff;
		uint32_t bufferLen;
		z_stream stream;
		virtual ssize_t writeBytes(const void *data, size_t len) {(void)data;(void)len;return 0;};

	public:
		OutputStream();
		~OutputStream();
		ssize_t write(const void *data, size_t len);
		bool flush();
		bool writeInt8(int8_t val);
		bool writeUInt8(uint8_t val);
		bool writeInt16(int16_t val);
		bool writeUInt16(uint16_t val);
		bool writeInt32(int32_t val);
		bool writeUInt32(uint32_t val);
		bool writeInt64(int64_t val);
		bool writeUInt64(uint64_t val);

	};

}

#endif
