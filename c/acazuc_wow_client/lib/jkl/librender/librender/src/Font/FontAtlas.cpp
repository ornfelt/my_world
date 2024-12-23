#include "FontAtlas.h"
#include <cstring>

#define GROW_SIZE 1024

namespace librender
{

	FontAtlas::FontAtlas(bool alphaTexture)
	: revision(1)
	, width(1024)
	, height(0)
	, bpp(alphaTexture ? 8 : 32)
	, alphaTexture(alphaTexture)
	{
	}

	FontAtlas::~FontAtlas()
	{
	}

	Glyph *FontAtlas::getGlyph(uint32_t codepoint)
	{
		std::unordered_map<uint32_t, Glyph>::iterator iter = this->glyphs.find(codepoint);
		if (iter == this->glyphs.end())
			return nullptr;
		return &iter->second;
	}

	Glyph *FontAtlas::addGlyph(uint32_t x, uint32_t y, uint32_t codepoint, Glyph glyph, const uint8_t *bitmap)
	{
		if (getGlyph(codepoint))
			return nullptr;
		if (this->alphaTexture)
		{
			size_t idx = y * this->width + x;
			for (uint32_t tY = 0; tY < glyph.getHeight(); ++tY)
			{
				std::memcpy(&this->data[idx], &bitmap[tY * glyph.getWidth()], glyph.getWidth());
				idx += this->width;
			}
		}
		else
		{
			for (uint32_t tY = 0; tY < glyph.getHeight(); ++tY)
			{
				for (uint32_t tX = 0; tX < glyph.getWidth(); ++tX)
				{
					((uint32_t*)this->data.data())[(y + tY) * this->width + x + tX] = 0xFFFFFF | ((uint32_t)bitmap[tY * glyph.getWidth() + tX] << 24);
				}
			}
		}
		Glyph *ret = &this->glyphs.emplace(codepoint, glyph).first->second;
		this->revision++;
		return ret;
	}


	bool FontAtlas::findPlace(uint32_t width, uint32_t height, uint32_t *x, uint32_t *y)
	{
		if (width > this->width || height > this->height)
			return false;
		for (uint32_t i = 0; i < this->lines.size(); ++i)
		{
			FontAtlasLine &line = this->lines[i];
			if (line.height >= height)
			{
				if (this->width - line.width >= width)
				{
					*x = line.width;
					*y = line.y;
					line.width += width;
					if (height > line.height)
						line.height = height;
					return true;
				}
				continue;
			}
			if (this->width - line.width < width)
				continue;
			if (i != this->lines.size() - 1)
				continue;
			if (this->height - line.y < height)
				continue;
			line.height = height;
			*x = line.width;
			*y = line.y;
			line.width += width;
			if (height > line.height)
				line.height = height;
			return true;
		}
		uint32_t newY = this->lines.size() ? this->lines.back().y + this->lines.back().height : 0;
		if (height > this->height - newY)
			return false;
		this->lines.resize(this->lines.size() + 1);
		this->lines.back().y = newY;
		this->lines.back().width = width;
		this->lines.back().height = height;
		*x = 0;
		*y = newY;
		return true;
	}

	void FontAtlas::grow()
	{
		this->height += GROW_SIZE;
		this->data.resize(this->width * this->height * this->bpp / 8);
	}

}
