#ifndef LIBRENDER_VIDEO_MODE_H
# define LIBRENDER_VIDEO_MODE_H

# include <cstdint>

namespace librender
{

	class VideoMode
	{

	private:
		int32_t refreshRate;
		int32_t redBits;
		int32_t greenBits;
		int32_t blueBits;
		int32_t width;
		int32_t height;

	public:
		VideoMode(int32_t refreshRate, int32_t redBits, int32_t greenBits, int32_t blueBits, int32_t width, int32_t height);
		inline int32_t getRefreshRate() {return this->refreshRate;};
		inline int32_t getRedBits() {return this->redBits;};
		inline int32_t getGreenBits() {return this->greenBits;};
		inline int32_t getBlueBits() {return this->blueBits;};
		inline int32_t getWidth() {return this->width;};
		inline int32_t getHeight() {return this->height;};

	};

}

#endif
