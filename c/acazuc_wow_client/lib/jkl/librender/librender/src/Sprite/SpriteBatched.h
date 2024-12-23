#ifndef LIBRENDER_SPRITE_BATCHED_H
# define LIBRENDER_SPRITE_BATCHED_H

# include "../DrawableBatched.h"
# include "./SpriteBase.h"

namespace librender
{

	class SpriteBatch;

	class SpriteBatched : public DrawableBatched, public SpriteBase
	{

	public:
		Texture *getTexture() const;
		void setParent(DrawableBatch *parent);

	};

}

#endif
