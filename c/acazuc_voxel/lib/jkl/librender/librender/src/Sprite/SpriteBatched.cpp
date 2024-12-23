#include "SpriteBatched.h"
#include "../DrawableBuffers.h"
#include "./SpriteBatch.h"

namespace librender
{

	void SpriteBatched::setParent(DrawableBatch *parent)
	{
		if (this->parent && (!parent || static_cast<SpriteBatch*>(this->parent)->getTexture() != static_cast<SpriteBatch*>(parent)->getTexture()))
			requireUpdates(DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_COLORS);
		DrawableBatched::setParent(parent);
	}

	Texture *SpriteBatched::getTexture() const
	{
		if (this->parent)
			return static_cast<SpriteBatch*>(this->parent)->getTexture();
		return nullptr;
	}

}
