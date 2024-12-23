#include "Font.h"
#include "../GL.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace librender
{

	Font::Font(FontModel &parent, uint32_t size, bool alphaTexture)
	: FontBase(parent, size, alphaTexture)
	, revision(1)
	, lastAtlasRevision(0)
	{
		this->texture.bind();
		this->texture.setFilter(TEXTURE_FILTER_LINEAR, TEXTURE_FILTER_LINEAR);
		this->texture.setWrap(TEXTURE_WRAP_CLAMP_TO_BORDER, TEXTURE_WRAP_CLAMP_TO_BORDER);
	}

	Font::~Font()
	{
	}

	void Font::bind()
	{
		this->texture.bind();
	}

	void Font::glUpdate()
	{
		if (this->atlas.getRevision() == this->lastAtlasRevision)
			return;
		this->lastAtlasRevision = this->atlas.getRevision();
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, this->atlas.isAlphaTexture() ? GL_ALPHA : GL_RGBA, this->atlas.getWidth(), this->atlas.getHeight(), 0, this->atlas.isAlphaTexture() ? GL_ALPHA : GL_RGBA, GL_UNSIGNED_BYTE, this->atlas.getData());
		this->revision++;
	}

}
