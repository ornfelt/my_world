#ifndef GZ_MEMORY_INPUT_STREAM
# define GZ_MEMORY_INPUT_STREAM

# include "./InputStream.h"
# include <cstdint>
# include <string>

namespace gz
{

	class MemoryInputStream : public InputStream
	{

	private:
		size_t len;
		size_t pos;
		void *data;
		ssize_t readBytes(void *data, size_t len);

	public:
		MemoryInputStream(void *data, size_t len);
		~MemoryInputStream();
		bool eof();

	};

}

#endif
