#ifndef GZ_MEMORY_OUTPUT_STREAM_H
# define GZ_MEMORY_OUTPUT_STREAM_H

# include "./OutputStream.h"
# include <cstdint>
# include <vector>
# include <string>

namespace gz
{

	class MemoryOutputStream : public OutputStream
	{

	private:
		std::vector<uint8_t> data;
		ssize_t writeBytes(const void *data, size_t len);

	public:
		MemoryOutputStream(uint8_t level = 6);
		~MemoryOutputStream();
		std::vector<uint8_t> &getData() {return this->data;};

	};

}

#endif
