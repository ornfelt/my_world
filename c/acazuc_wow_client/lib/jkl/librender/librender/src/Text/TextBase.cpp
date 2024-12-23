#include "TextBase.h"
#include "./DrawableBuffers.h"
#include "../GL.h"
#include <libunicode/utf8.h>

namespace librender
{

	TextBase::TextBase()
	: shadowColor(Color::BLACK)
	, topColor(Color::WHITE)
	, botColor(Color::WHITE)
	, fontRevision(0)
	, charsNumber(0)
	, shadowSize(0)
	, lineHeight(-1)
	, maxWidth(-1)
	, shadowX(0)
	, shadowY(0)
	, height(0)
	, width(0)
	, mustCalcHeight(false)
	, mustCalcWidth(false)
	{
	}

	size_t TextBase::getShadowLen()
	{
		if (this->shadowSize <= 0)
			return 0;
		if (this->shadowSize == 1)
			return 1;
		size_t tmp = this->shadowSize - 1;
		size_t tmp2 = 1 + tmp * 2;
		return tmp2 * tmp2 - 1 - 4 * tmp;
	}

	void TextBase::updateTexCoords()
	{
		char *iter = const_cast<char*>(this->text.c_str());
		char *end = iter + this->text.length();
		Font *font = getFont();
		for (size_t i = 0; i < this->charsNumber; ++i)
		{
			uint32_t character = utf8::next(iter, end);
			font->charTexCoords(character, reinterpret_cast<float*>(&this->texCoords[i * 4]));
		}
		size_t shadowLen = getShadowLen();
		size_t copyCount = this->charsNumber * 4;
		for (size_t i = 0; i < shadowLen; ++i)
			std::copy(this->texCoords.begin(), this->texCoords.begin() + copyCount, this->texCoords.begin() + this->charsNumber * 4 * (i + 1));
	}

	void TextBase::updatePositions()
	{
		size_t shadowLen = getShadowLen();
		float x = 0;
		float y = 0;
		char *iter = const_cast<char*>(this->text.c_str());
		char *end = iter + this->text.length();
		int32_t index = (shadowLen * this->charsNumber) * 4;
		Font *font = getFont();
		for (size_t i = 0; i < this->charsNumber; ++i)
		{
			uint32_t character = utf8::next(iter, end);
			if (character == '\n')
			{
				y += getLineHeight();
				x = 0;
				std::fill(this->positions.begin() + index, this->positions.begin() + index + 4, 0);
				index += 4;
				continue;
			}
			Glyph *glyph = font->getGlyph(character);
			if (!glyph)
			{
				std::fill(this->positions.begin() + index, this->positions.begin() + index + 4, 0);
				index += 4;
				continue;
			}
			else
			{
				float charWidth = glyph->getAdvance();
				if (this->maxWidth > 0 && x && x + charWidth >= this->maxWidth)
				{
					y += getLineHeight();
					x = 0;
				}
				float charRenderLeft = x + glyph->getOffsetX();
				float charRenderTop = y + glyph->getOffsetY();
				float charRenderRight = charRenderLeft + glyph->getWidth();
				float charRenderBottom = charRenderTop + glyph->getHeight();
				this->positions[index].x = charRenderLeft;
				this->positions[index].y = charRenderTop;
				++index;
				this->positions[index].x = charRenderRight;
				this->positions[index].y = charRenderTop;
				++index;
				this->positions[index].x = charRenderRight;
				this->positions[index].y = charRenderBottom;
				++index;
				this->positions[index].x = charRenderLeft;
				this->positions[index].y = charRenderBottom;
				++index;
				x += charWidth;
			}
		}
		if (!shadowLen)
			return;
		size_t tmp = 1 + (this->shadowSize - 1) * 2;
		uint8_t arrCount = 0;
		size_t tmp2 = shadowLen * this->charsNumber * 4;
		size_t tmptmp = tmp * tmp;
		for (size_t i = 0; i < tmptmp; ++i)
		{
			int32_t sx = i % tmp - (this->shadowSize - 1);
			int32_t sy = i / tmp - (this->shadowSize - 1);
			if (std::abs(sx) == std::abs(sy) && this->shadowSize != 1)
				continue;
			size_t index = this->charsNumber * 4 * arrCount;
			Vec2 vtmp(this->shadowX + sx, this->shadowY + sy);
			for (size_t j = 0; j < this->charsNumber * 4; ++j)
				this->positions[index + j] = this->positions[tmp2 + j] + vtmp;
			arrCount++;
		}
	}

	void TextBase::updateIndices()
	{
		size_t count = 0;
		GLuint currentIndice = 0;
		for (size_t i = 0; i < this->indicesNumber / 6; ++i)
		{
			indices[count++] = currentIndice + 0;
			indices[count++] = currentIndice + 3;
			indices[count++] = currentIndice + 1;
			indices[count++] = currentIndice + 2;
			indices[count++] = currentIndice + 1;
			indices[count++] = currentIndice + 3;
			currentIndice += 4;
		}
	}

	void TextBase::updateColors()
	{
		size_t shadowLen = getShadowLen();
		{
			char *iter = const_cast<char*>(this->text.c_str());
			char *end = iter + this->text.length();
			size_t tmp = shadowLen * this->charsNumber * 4;
			float lineHeight = getFont()->getLineHeight();
			Color diff = Color(this->botColor - this->topColor);
			Font *font = getFont();
			for (size_t i = 0; i < this->charsNumber; i++)
			{
				uint32_t character = utf8::next(iter, end);
				Glyph *glyph = font->getGlyph(character);
				Color topColor;
				Color botColor;
				if (glyph)
				{
					float topFactor = glyph->getOffsetY() / lineHeight;
					topFactor = std::min(1.f, std::max(0.f, topFactor));
					float botFactor = (glyph->getOffsetY() + glyph->getHeight()) / lineHeight;
					botFactor = std::min(1.f, std::max(0.f, botFactor));
					topColor = Color(this->topColor + diff * topFactor);
					botColor = Color(this->topColor + diff * botFactor);
				}
				else
				{
					topColor = this->topColor;
					botColor = this->botColor;
				}
				this->colors[tmp++] = topColor;
				this->colors[tmp++] = topColor;
				this->colors[tmp++] = botColor;
				this->colors[tmp++] = botColor;
			}
		}
		if (!shadowLen)
			return;
		for (size_t i = 0; i < this->charsNumber * 4; ++i)
			this->colors[i] = this->shadowColor;
		for (size_t i = 1; i < shadowLen; ++i)
			std::copy(this->colors.begin(), this->colors.begin() + this->charsNumber * 4, this->colors.begin() + this->charsNumber * i * 4);
	}

	void TextBase::updateBuffers()
	{
		Font *font = getFont();
		if (!font)
			return;
		if (this->updatesRequired & (DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS))
		{
			//Hack fix, load all glyphs before update
			char *iter = const_cast<char*>(this->text.c_str());
			char *end = iter + this->text.length();
			for (size_t i = 0; i < this->charsNumber; ++i)
			{
				uint32_t character = utf8::next(iter, end);
				font->getGlyph(character);
			}
		}
		font->glUpdate();
		if (font->getRevision() != this->fontRevision)
		{
			this->fontRevision = font->getRevision();
			requireUpdates(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_COLORS);
		}
		DrawableBase::updateBuffers();
	}

	void TextBase::setText(const std::string &text)
	{
		recalcWidth();
		recalcHeight();
		size_t newLen = utf8::distance(text.begin(), text.end());
		if (this->charsNumber != newLen)
		{
			this->charsNumber = newLen;
			requireUpdates(DRAWABLE_BUFFER_INDICES);
			size_t shadowLen = getShadowLen();
			resize(this->charsNumber * 4 * (1 + shadowLen), this->charsNumber * 6 * (1 + shadowLen));
		}
		this->text = text;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_COLORS);
	}

	void TextBase::setShadowColor(Color color)
	{
		if (color == this->shadowColor)
			return;
		this->shadowColor = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TextBase::setTopColor(Color color)
	{
		if (color == this->topColor)
			return;
		this->topColor = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TextBase::setBotColor(Color color)
	{
		if (color == this->botColor)
			return;
		this->botColor = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TextBase::setColor(Color color)
	{
		setTopColor(color);
		setBotColor(color);
	}

	void TextBase::setShadowSize(int16_t shadowSize)
	{
		if (this->shadowSize == shadowSize)
			return;
		this->shadowSize = shadowSize;
		size_t shadowLen = getShadowLen();
		resize(this->charsNumber * 4 * (1 + shadowLen), this->charsNumber * 6 * (1 + shadowLen));
	}

	void TextBase::setShadowX(int32_t shadowX)
	{
		if (this->shadowX == shadowX)
			return;
		this->shadowX = shadowX;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void TextBase::setShadowY(int32_t shadowY)
	{
		if (this->shadowY == shadowY)
			return;
		this->shadowY = shadowY;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void TextBase::setMaxWidth(int32_t maxWidth)
	{
		if (this->maxWidth == maxWidth)
			return;
		this->maxWidth = maxWidth;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
		recalcWidth();
		recalcHeight();
	}

	int32_t TextBase::getWidth()
	{
		if (this->mustCalcWidth)
		{
			if (!getFont())
				this->width = 0;
			else
				this->width = getFont()->getWidth(this->text);
			if (this->maxWidth > 0 && this->width > this->maxWidth)
				this->width = this->maxWidth;
			this->mustCalcWidth = false;
		}
		return this->width * this->scale.x;
	}

	int32_t TextBase::getHeight()
	{
		if (!this->mustCalcHeight)
			return this->height * this->scale.y;
		this->mustCalcHeight = false;
		this->height = 0;
		if (!getFont())
			return this->height * this->scale.y;
		this->height = getLineHeight();
		char *iter = const_cast<char*>(this->text.c_str());
		char *end = iter + this->text.length();
		float x = 0;
		for (size_t i = 0; i < this->charsNumber; ++i)
		{
			uint32_t character = utf8::next(iter, end);
			if (character == '\n')
			{
				x = 0;
				this->height += getLineHeight();
				continue;
			}
			Glyph *glyph = getFont()->getGlyph(character);
			if (!glyph)
				continue;
			x += glyph->getAdvance();
			if (this->maxWidth > 0 && x >= this->maxWidth)
			{
				this->height += getLineHeight();
				x = 0;
			}
		}
		return this->height * this->scale.y;
	}

	int32_t TextBase::getLineHeight()
	{
		if (this->lineHeight == -1)
		{
			Font *font = getFont();
			if (!font)
				return 0;
			return font->getLineHeight() * this->scale.y;
		}
		return this->lineHeight * this->scale.y;
	}

}
