#include "TextBatch.h"
#include "../DrawableBuffers.h"
#include "./TextBatched.h"
#include "../GL.h"

namespace librender
{

	TextBatch::TextBatch()
	: font(nullptr)
	{
	}

	void TextBatch::draw()
	{
		if (!this->font)
			return;
		if (!update())
			return;
		this->font->bind();
		DrawableBatch::draw();
	}

	int32_t TextBatch::getLineHeight()
	{
		if (!this->font)
			return 0;
		return this->font->getLineHeight();
	}

	void TextBatch::setFont(Font *font)
	{
		this->font = font;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_POSITIONS);
		for (size_t i = 0; i < this->childs.size(); ++i)
		{
			TextBatched *child = static_cast<TextBatched*>(this->childs[i]);
			child->addChanges(DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_POSITIONS);
			child->recalcWidth();
			child->recalcHeight();
		}
	}

}
