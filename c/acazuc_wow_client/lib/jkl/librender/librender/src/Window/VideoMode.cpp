#include "VideoMode.h"

namespace librender
{

	VideoMode::VideoMode(int32_t refreshRate, int32_t redBits, int32_t greenBits, int32_t blueBits, int32_t width, int32_t height)
	: refreshRate(refreshRate)
	, redBits(redBits)
	, greenBits(greenBits)
	, blueBits(blueBits)
	, width(width)
	, height(height)
	{
	}

}
