#include "FileInputStream.h"
#include <cstring>

namespace gz
{

	FileInputStream::FileInputStream()
	: openedFile(false)
	, opened(false)
	, file(NULL)
	{
		//Empty
	}

	FileInputStream::~FileInputStream()
	{
		close();
	}

	bool FileInputStream::open(std::string filename)
	{
		if (this->opened || this->file)
			return false;
		std::memset(&this->stream, 0, sizeof(this->stream));
		if (inflateInit(&this->stream) != Z_OK)
			return false;
		if (!(this->file = std::fopen(filename.c_str(), "rb")))
		{
			inflateEnd(&this->stream);
			return false;
		}
		this->opened = true;
		this->openedFile = true;
		return true;
	}

	bool FileInputStream::open(FILE *file)
	{
		if (this->opened || this->file)
			return false;
		std::memset(&this->stream, 0, sizeof(this->stream));
		if (inflateInit(&this->stream) != Z_OK)
			return false;
		this->file = file;
		this->opened = true;
		this->openedFile = true;
		return true;
	}

	void FileInputStream::close()
	{
		if (this->openedFile && this->file)
		{
			std::fclose(this->file);
			this->openedFile = false;
			this->file = NULL;
		}
		if (this->opened)
		{
			inflateEnd(&this->stream);
			this->opened = false;
		}
	}

	ssize_t FileInputStream::readBytes(void *data, size_t len)
	{
		if (!this->opened || !this->file)
			return -1;
		return std::fread(data, 1, len, this->file);
	}

	bool FileInputStream::eof()
	{
		if (!this->opened || !this->file)
			return true;
		return std::feof(this->file);
	}

}
