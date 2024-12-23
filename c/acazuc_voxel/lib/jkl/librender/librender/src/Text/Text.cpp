#include "Text.h"
#include "../DrawableBuffers.h"
#include "../GL.h"

namespace librender
{

	Text::Text()
	: font(nullptr)
	{
	}

	void Text::draw()
	{
		if (!this->font)
			return;
		updateBuffers();
		this->font->bind();
		Drawable::draw();
	}

	void Text::setFont(Font *font)
	{
		if (this->font == font)
			return;
		this->font = font;
		this->lineHeight = this->font->getLineHeight();
		requireUpdates(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS);
		recalcWidth();
		recalcHeight();
	}

}
