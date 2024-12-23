#include "OldACI.h"
#include <libgzip/FileOutputStream.h>
#include <libgzip/FileInputStream.h>
#include <cstdio>

namespace libformat
{

	bool OldACI::read(std::string file, char *&data, uint32_t &width, uint32_t &height)
	{
		gz::FileInputStream stream;
		if (!stream.open(file))
			goto error1;
		char tmp[4];
		if (stream.read(tmp, 4) != 4)
			goto error2;
		width = ((127 - (tmp[0])) << 24) | ((127 - (tmp[1])) << 16) | ((127 - (tmp[2])) << 8) | (127 - (tmp[3]));
		if (stream.read(tmp, 4) != 4)
			goto error2;
		height = ((127 - (tmp[0])) << 24) | ((127 - (tmp[1])) << 16) | ((127 - (tmp[2])) << 8) | (127 - (tmp[3]));
		data = new char[width * height * 4];
		if (stream.read(data, width * height * 4) != width * height * 4)
			goto error2;
		stream.close();
		return true;
	error2:
		delete[] (data);
	error1:
		return false;
	}

	bool OldACI::write(std::string file, char *data, uint32_t width, uint32_t height)
	{
		gz::FileOutputStream stream;
		char tmp[4];
		if (!stream.open(file))
			goto error1;
		tmp[0] = 127 - ((width >> 24) & 0xFF);
		tmp[1] = 127 - ((width >> 16) & 0xFF);
		tmp[2] = 127 - ((width >> 8) & 0xFF);
		tmp[3] = 127 - (width & 0xFF);
		if (stream.write(tmp, 4) != 4)
			goto error1;
		tmp[0] = 127 - ((height >> 24) & 0xFF);
		tmp[1] = 127 - ((height >> 16) & 0xFF);
		tmp[2] = 127 - ((height >> 8) & 0xFF);
		tmp[3] = 127 - (height & 0xFF);
		if (stream.write(tmp, 4) != 4)
			goto error1;
		if (stream.write(data, width * height * 4) != width * height * 4)
			goto error1;
		stream.close();
		return true;
	error1:
		return false;
	}

}
