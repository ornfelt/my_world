#include "./TextBatched.h"
#include "../DrawableBuffers.h"
#include "./TextBatch.h"

namespace librender
{

	void TextBatched::setParent(DrawableBatch *parent)
	{
		if (this->parent && (!parent || static_cast<TextBatch*>(this->parent)->getFont() != static_cast<TextBatch*>(parent)->getFont()))
		{
			requireUpdates(DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_COLORS);
			recalcWidth();
			recalcHeight();
		}
		DrawableBatched::setParent(parent);
	}

	Font *TextBatched::getFont()
	{
		if (this->parent)
			return static_cast<TextBatch*>(this->parent)->getFont();
		return nullptr;
	}

}
