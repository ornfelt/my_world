#include "FontBase.h"
#include <libunicode/utf8.h>
#include <algorithm>

namespace librender
{

	FontBase::FontBase(FontModel &parent, uint32_t size, bool alphaTexture)
	: parent(parent)
	, atlas(alphaTexture)
	, height(0)
	, size(size)
	{
		if (!this->parent.setSize(this->size))
			throw std::exception();
		this->height = this->parent.getFtFace()->size->metrics.height >> 6;
		if (this->height > 2)
			this->height -= 2;
	}

	FontBase::~FontBase()
	{
	}

	Glyph *FontBase::loadGlyph(uint32_t codepoint)
	{
		if (!this->parent.setSize(this->size))
			return nullptr;
		if (codepoint < 0x1f) /* control code */
			return nullptr;
		uint32_t index = FT_Get_Char_Index(this->parent.getFtFace(), codepoint);
		if (!index)
			return nullptr;
		if (FT_Load_Glyph(this->parent.getFtFace(), index, FT_LOAD_DEFAULT))
			return nullptr;
		if (!this->parent.getFtFace()->glyph->advance.x)
			return nullptr;
		if (FT_Render_Glyph(this->parent.getFtFace()->glyph, FT_RENDER_MODE_NORMAL))
			return nullptr;
		Glyph glyph(this->parent.getFtFace()->glyph->advance.x >> 6, \
				this->parent.getFtFace()->glyph->bitmap.width, this->parent.getFtFace()->glyph->bitmap.rows, \
				this->parent.getFtFace()->glyph->bitmap_left, this->size - this->parent.getFtFace()->glyph->bitmap_top);
		uint32_t glyphX;
		uint32_t glyphY;
		if (this->atlas.findPlace(glyph.getWidth() + 1, glyph.getHeight() + 1, &glyphX, &glyphY))
		{
			glyph.setTexX(glyphX + 1);
			glyph.setTexY(glyphY + 1);
			return this->atlas.addGlyph(glyphX + 1, glyphY + 1, codepoint, glyph, this->parent.getFtFace()->glyph->bitmap.buffer);
		}
		this->atlas.grow();
		if (this->atlas.findPlace(glyph.getWidth() + 1, glyph.getHeight() + 1, &glyphX, &glyphY))
		{
			glyph.setTexX(glyphX + 1);
			glyph.setTexY(glyphY + 1);
			return this->atlas.addGlyph(glyphX + 1, glyphY + 1, codepoint, glyph, this->parent.getFtFace()->glyph->bitmap.buffer);
		}
		glyph.setWidth(0);
		glyph.setHeight(0);
		glyph.setTexX(0);
		glyph.setTexY(0);
		return this->atlas.addGlyph(0, 0, codepoint, glyph, nullptr);
	}

	int32_t FontBase::getWidth(const std::string &text)
	{
		uint32_t maxWidth = 0;
		uint32_t currentWidth = 0;
		const char *iter = text.c_str();
		const char *end = iter + text.length();
		while (iter != end)
		{
			uint32_t currentChar = utf8::next(iter, end);
			if (currentChar == '\n')
			{
				if (currentWidth > maxWidth)
					maxWidth = currentWidth;
				currentWidth = 0;
			}
			else
			{
				Glyph *glyph = getGlyph(currentChar);
				if (glyph)
					currentWidth += glyph->getAdvance();
			}
		}
		if (currentWidth > maxWidth)
			maxWidth = currentWidth;
		return maxWidth;
	}

	int32_t FontBase::getHeight(const std::string &text)
	{
		uint32_t nlNb = 1;
		size_t pos = 0;
		while ((pos = text.find('\n', pos)) != std::string::npos)
		{
			++pos;
			++nlNb;
		}
		return this->height * nlNb;
	}

	Glyph *FontBase::getGlyph(uint32_t codepoint)
	{
		Glyph *glyph = this->atlas.getGlyph(codepoint);
		if (glyph)
			return glyph;
		glyph = loadGlyph(codepoint);
		if (glyph)
			return glyph;
		Glyph tmp(0, 0, 0, 0, 0);
		tmp.setTexX(0);
		tmp.setTexY(0);
		return this->atlas.addGlyph(0, 0, codepoint, tmp, nullptr);
	}

	void FontBase::texCoords(int32_t texX, int32_t texY, int32_t texWidth, int32_t texHeight, float *texCoords)
	{
		float textureSrcX = static_cast<float>(texX) / this->atlas.getWidth();
		float textureSrcY = static_cast<float>(texY) / this->atlas.getHeight();
		float renderWidth = static_cast<float>(texWidth) / this->atlas.getWidth();
		float renderHeight = static_cast<float>(texHeight) / this->atlas.getHeight();
		texCoords[0] = textureSrcX;
		texCoords[1] = textureSrcY;
		texCoords[2] = textureSrcX + renderWidth;
		texCoords[3] = textureSrcY;
		texCoords[4] = textureSrcX + renderWidth;
		texCoords[5] = textureSrcY + renderHeight;
		texCoords[6] = textureSrcX;
		texCoords[7] = textureSrcY + renderHeight;
	}

	void FontBase::charTexCoords(uint32_t codepoint, float *texCoords)
	{
		glyphTexCoords(getGlyph(codepoint), texCoords);
	}

	void FontBase::glyphTexCoords(Glyph *glyph, float *coords)
	{
		if (!glyph)
		{
			std::fill(coords, coords + 8, 0);
			return;
		}
		texCoords(glyph->getTexX(), glyph->getTexY(), glyph->getWidth(), glyph->getHeight(), coords);
	}

}
