#include "TextTessellator.h"
#include "../GL.h"

namespace librender
{

	TextTessellator::TextTessellator()
	: font(nullptr)
	{
	}

	void TextTessellator::draw()
	{
		if (!this->font)
			return;
		this->font->bind();
		DrawableTessellator::draw();
	}

	void TextTessellator::addGlyph(Glyph *glyph, Vec2 pos, Vec4 color)
	{
		if (!this->font)
			return;
		if (!glyph)
			return;
		Vec2 tmp[4];
		this->font->glyphTexCoords(glyph, &tmp[0].x);
		Vec2 org(pos);
		Vec2 dst(pos + Vec2(glyph->getWidth(), glyph->getHeight()));
		uint32_t base = this->positions.size();
		addPoint(Vec2(org.x, org.y), tmp[0], color);
		addPoint(Vec2(dst.x, org.y), tmp[1], color);
		addPoint(Vec2(dst.x, dst.y), tmp[2], color);
		addPoint(Vec2(org.x, dst.y), tmp[3], color);
		this->indices.push_back(base + 0);
		this->indices.push_back(base + 3);
		this->indices.push_back(base + 1);
		this->indices.push_back(base + 2);
		this->indices.push_back(base + 1);
		this->indices.push_back(base + 3);
	}

	void TextTessellator::addChar(uint32_t character, Vec2 pos, Vec4 color)
	{
		if (!this->font)
			return;
		addGlyph(this->font->getGlyph(character), pos, color);
	}

}
