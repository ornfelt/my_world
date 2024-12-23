#include "Glyph.h"

namespace librender
{

	Glyph::Glyph(uint16_t advance, uint16_t width, uint16_t height, int16_t offsetX, int16_t offsetY)
	: advance(advance)
	, height(height)
	, width(width)
	, offsetX(offsetX)
	, offsetY(offsetY)
	, texX(0)
	, texY(0)
	{
	}

}
