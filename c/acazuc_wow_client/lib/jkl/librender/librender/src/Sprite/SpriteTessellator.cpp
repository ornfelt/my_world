#include "SpriteTessellator.h"
#include "../DrawableBuffers.h"
#include "../GL.h"

namespace librender
{

	SpriteTessellator::SpriteTessellator()
	: texture(nullptr)
	{
	}

	void SpriteTessellator::draw()
	{
		if (this->texture)
		{
			this->buffers |= DRAWABLE_BUFFER_TEX_COORDS;
			this->texture->bind();
		}
		else
		{
			this->buffers &= ~DRAWABLE_BUFFER_TEX_COORDS;
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		DrawableTessellator::draw();
	}

	void SpriteTessellator::addQuad(Vec2 pos, Vec2 size, Vec2 texPos, Vec2 texSize, Vec4 color)
	{
		Vec2 org(pos);
		Vec2 dst(pos + size);
		Vec2 texOrg(texPos);
		Vec2 texDst(texPos + texSize);
		uint32_t base = this->positions.size();
		addPoint(Vec2(org.x, org.y), Vec2(texOrg.x, texOrg.y), color);
		addPoint(Vec2(dst.x, org.y), Vec2(texDst.x, texOrg.y), color);
		addPoint(Vec2(dst.x, dst.y), Vec2(texDst.x, texDst.y), color);
		addPoint(Vec2(org.x, dst.y), Vec2(texOrg.x, texDst.y), color);
		this->indices.push_back(base + 0);
		this->indices.push_back(base + 3);
		this->indices.push_back(base + 1);
		this->indices.push_back(base + 2);
		this->indices.push_back(base + 1);
		this->indices.push_back(base + 3);
	}

}
