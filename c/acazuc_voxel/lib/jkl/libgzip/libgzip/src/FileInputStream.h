#ifndef GZ_FILE_INPUT_STREAM
# define GZ_FILE_INPUT_STREAM

# include "./InputStream.h"
# include <cstdint>
# include <cstdio>
# include <string>

namespace gz
{

	class FileInputStream : public InputStream
	{

	private:
		bool openedFile;
		bool opened;
		FILE *file;
		ssize_t readBytes(void *data, size_t len);

	public:
		FileInputStream();
		~FileInputStream();
		bool open(std::string filename);
		bool open(FILE *file);
		void close();
		bool eof();

	};

}

#endif
