#include "ShaderTextEntry.h"
#include "./ShaderTextUpdate.h"
#include <cstring>
#include <libunicode/utf8.h>

namespace librender
{

	ShaderTextEntry::ShaderTextEntry()
	: shadowColor(Color::BLACK)
	, color(Color::WHITE)
	, scale(1)
	, pos(0)
	, verticesNumber(0)
	, fontRevision(0)
	, charsNumber(0)
	, updatesRequired(0)
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

	ShaderTextEntry::~ShaderTextEntry()
	{
	}

	uint32_t ShaderTextEntry::getShadowLen()
	{
		if (this->shadowSize <= 0)
			return 0;
		if (this->shadowSize == 1)
			return 1;
		int32_t tmp = this->shadowSize - 1;
		int32_t tmp2 = 1 + tmp * 2;
		return tmp2 * tmp2 - 1 - 4 * tmp;
	}

	void ShaderTextEntry::fillTexCoords(Vec2 *texCoords)
	{
		const char *iter = this->text.c_str();
		const char *end = iter + this->text.length();
		for (uint32_t i = 0; i < this->charsNumber; ++i)
		{
			uint32_t character = utf8::next(iter, end);
			getFont()->charTexCoords(character, reinterpret_cast<float*>(&texCoords[i * 4]));
		}
		uint32_t shadowLen = getShadowLen();
		uint32_t copyCount = this->charsNumber * 4 * sizeof(*texCoords);
		for (uint32_t i = 0; i < shadowLen; ++i)
			std::memcpy(&texCoords[this->charsNumber * 4 * (i + 1)], &texCoords[0], copyCount);
	}

	void ShaderTextEntry::fillVertexes(Vec2 *vertexes)
	{
		uint32_t shadowLen = getShadowLen();
		float x = 0;
		float y = 0;
		const char *iter = this->text.c_str();
		const char *end = iter + this->text.length();
		int32_t index = (shadowLen * this->charsNumber) * 4;
		for (uint32_t i = 0; i < this->charsNumber; ++i)
		{
			uint32_t character = utf8::next(iter, end);
			if (character == '\n')
			{
				y += getLineHeight();
				x = 0;
				std::memset(&vertexes[index], 0, 4 * sizeof(*this->vertexes.data()));
				index += 4;
				continue;
			}
			Glyph *glyph = getFont()->getGlyph(character);
			if (!glyph)
			{
				std::memset(&vertexes[index], 0, 4 * sizeof(*this->vertexes.data()));
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
				vertexes[index].x = charRenderLeft;
				vertexes[index].y = charRenderTop;
				++index;
				vertexes[index].x = charRenderRight;
				vertexes[index].y = charRenderTop;
				++index;
				vertexes[index].x = charRenderRight;
				vertexes[index].y = charRenderBottom;
				++index;
				vertexes[index].x = charRenderLeft;
				vertexes[index].y = charRenderBottom;
				++index;
				x += charWidth;
			}
		}
		if (!shadowLen)
			return;
		uint32_t tmp = 1 + (this->shadowSize - 1) * 2;
		uint8_t arrCount = 0;
		uint32_t tmp2 = shadowLen * this->charsNumber * 4;
		uint32_t tmptmp = tmp * tmp;
		for (uint32_t i = 0; i < tmptmp; ++i)
		{
			int32_t sx = i % tmp - (this->shadowSize - 1);
			int32_t sy = i / tmp - (this->shadowSize - 1);
			if (std::abs(sx) == std::abs(sy) && this->shadowSize != 1)
				continue;
			uint32_t index = this->charsNumber * 4 * arrCount;
			Vec2 vtmp(this->shadowX + sx, this->shadowY + sy);
			for (uint32_t j = 0; j < this->charsNumber * 4; ++j)
				vertexes[index + j] = vertexes[tmp2 + j] + vtmp;
			arrCount++;
		}
	}

	void ShaderTextEntry::fillColors(Vec4 *colors)
	{
		uint32_t shadowLen = getShadowLen();
		{
			int32_t tmp = shadowLen * this->charsNumber * 4;
			for (uint32_t i = 0; i < this->charsNumber * 4; ++i)
				colors[tmp++] = this->color;
		}
		if (!shadowLen)
			return;
		for (uint32_t i = 0; i < this->charsNumber * 4; ++i)
			colors[i] = this->shadowColor;
		for (uint32_t i = 1; i < shadowLen; ++i)
			std::memcpy(&colors[this->charsNumber * 4 * i], &colors[0], this->charsNumber * 4 * sizeof(*this->colors.data()));
	}

	void ShaderTextEntry::requireUpdates(uint8_t update)
	{
		this->updatesRequired |= update;
	}

	void ShaderTextEntry::update()
	{
		Font *font = getFont();
		if (!font)
			return;
		font->glUpdate();
		if (font->getRevision() != this->fontRevision)
		{
			this->fontRevision = font->getRevision();
			requireUpdates(SHADER_TEXT_UPDATE_VERTEXES | SHADER_TEXT_UPDATE_TEX_COORDS);
		}
		if (this->updatesRequired & SHADER_TEXT_UPDATE_TEX_COORDS)
			updateTexCoords();
		if (this->updatesRequired & SHADER_TEXT_UPDATE_VERTEXES)
			updateVertexes();
		if (this->updatesRequired & SHADER_TEXT_UPDATE_COLORS)
			updateColors();
		this->updatesRequired = 0;
	}

	void ShaderTextEntry::resize(uint32_t len)
	{
		requireUpdates(SHADER_TEXT_UPDATE_VERTEXES | SHADER_TEXT_UPDATE_TEX_COORDS | SHADER_TEXT_UPDATE_COLORS);
		this->charsNumber = len;
		this->verticesNumber = this->charsNumber * 4;
		this->verticesNumber *= 1 + getShadowLen();
		this->texCoords.resize(this->verticesNumber);
		this->vertexes.resize(this->verticesNumber);
		this->colors.resize(this->verticesNumber);
	}

	void ShaderTextEntry::setText(std::string &text)
	{
		recalcWidth();
		recalcHeight();
		uint32_t newLen = utf8::distance(text.begin(), text.end());
		if (this->charsNumber != newLen)
			resize(newLen);
		this->text = text;
		requireUpdates(SHADER_TEXT_UPDATE_VERTEXES | SHADER_TEXT_UPDATE_TEX_COORDS);
	}

	void ShaderTextEntry::setShadowColor(Color color)
	{
		if (color == this->shadowColor)
			return;
		this->shadowColor = color;
		requireUpdates(SHADER_TEXT_UPDATE_COLORS);
	}

	void ShaderTextEntry::setColor(Color color)
	{
		if (color == this->color)
			return;
		this->color = color;
		requireUpdates(SHADER_TEXT_UPDATE_COLORS);
	}

	void ShaderTextEntry::setShadowSize(int16_t shadowSize)
	{
		if (this->shadowSize == shadowSize)
			return;
		this->shadowSize = shadowSize;
		resize(this->charsNumber);
		requireUpdates(SHADER_TEXT_UPDATE_VERTEXES | SHADER_TEXT_UPDATE_TEX_COORDS | SHADER_TEXT_UPDATE_COLORS);
	}

	void ShaderTextEntry::setShadowX(int32_t shadowX)
	{
		if (this->shadowX == shadowX)
			return;
		this->shadowX = shadowX;
		requireUpdates(SHADER_TEXT_UPDATE_VERTEXES);
	}

	void ShaderTextEntry::setShadowY(int32_t shadowY)
	{
		if (this->shadowY == shadowY)
			return;
		this->shadowY = shadowY;
		requireUpdates(SHADER_TEXT_UPDATE_VERTEXES);
	}

	void ShaderTextEntry::setScaleX(float scaleX)
	{
		if (this->scale.x == scaleX)
			return;
		this->scale.x = scaleX;
	}

	void ShaderTextEntry::setScaleY(float scaleY)
	{
		if (this->scale.y == scaleY)
			return;
		this->scale.y = scaleY;
	}

	void ShaderTextEntry::setMaxWidth(int32_t maxWidth)
	{
		if (this->maxWidth == maxWidth)
			return;
		this->maxWidth = maxWidth;
		requireUpdates(SHADER_TEXT_UPDATE_VERTEXES);
		recalcWidth();
		recalcHeight();
	}

	int32_t ShaderTextEntry::getWidth()
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

	int32_t ShaderTextEntry::getHeight()
	{
		if (!this->mustCalcHeight)
			return this->height * this->scale.y;
		this->mustCalcHeight = false;
		this->height = 0;
		if (!getFont())
			return this->height * this->scale.y;
		this->height = getLineHeight();
		const char *iter = this->text.c_str();
		const char *end = iter + this->text.length();
		float x = 0;
		for (uint32_t i = 0; i < this->charsNumber; ++i)
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

	int32_t ShaderTextEntry::getLineHeight()
	{
		if (this->lineHeight == -1)
		{
			if (!getFont())
				return 0;
			return getFont()->getLineHeight() * this->scale.y;
		}
		return this->lineHeight * this->scale.y;
	}

}
