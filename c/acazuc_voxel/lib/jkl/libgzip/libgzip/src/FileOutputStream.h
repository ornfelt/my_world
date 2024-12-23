#ifndef GZ_FILE_OUTPUT_STREAM_H
# define GZ_FILE_OUTPUT_STREAM_H

# include "./OutputStream.h"
# include <cstdint>
# include <cstdio>
# include <string>

namespace gz
{

	class FileOutputStream : public OutputStream
	{

	private:
		FILE *file;
		bool openedFile;
		bool opened;
		ssize_t writeBytes(const void *data, size_t len);

	public:
		FileOutputStream();
		~FileOutputStream();
		bool open(std::string filename, uint8_t level = 6);
		bool open(FILE *file, uint8_t level = 6);
		void close();

	};

}

#endif
