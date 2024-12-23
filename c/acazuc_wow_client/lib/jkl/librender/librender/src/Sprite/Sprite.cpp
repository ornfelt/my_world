#include "Sprite.h"
#include "../DrawableBuffers.h"
#include "../GL.h"

namespace librender
{

	Sprite::Sprite()
	: texture(nullptr)
	{
	}

	void Sprite::draw()
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
		updateBuffers();
		Drawable::draw();
	}

	void Sprite::setTexture(Texture *texture)
	{
		if (this->texture == texture)
			return;
		this->texture = texture;
	}

}
