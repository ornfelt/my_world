#ifndef LIBRENDER_SPRITE_BATCH_H
# define LIBRENDER_SPRITE_BATCH_H

# include "../Texture/Texture.h"
# include "../DrawableBatch.h"

namespace librender
{

	class SpriteBatch : public DrawableBatch
	{

	private:
		Texture *texture;

	public:
		SpriteBatch();
		void draw();
		void setTexture(Texture *texture);
		inline Texture *getTexture() const {return this->texture;};
		int32_t getTextureWidth() const;
		int32_t getTextureHeight() const;

	};

}

#endif
