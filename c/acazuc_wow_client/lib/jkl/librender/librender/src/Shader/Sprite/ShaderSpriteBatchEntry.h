#ifndef LIBRENDER_SHADER_SPRITE_BATCH_ENTRY_H
# define LIBRENDER_SHADER_SPRITE_BATCH_ENTRY_H

# include "./ShaderSpriteEntry.h"

namespace librender
{

	class ShaderSpriteBatch;

	class ShaderSpriteBatchEntry : public ShaderSpriteEntry
	{

	private:
		ShaderSpriteBatch *parent;
		uint8_t changes;

	public:
		ShaderSpriteBatchEntry();
		~ShaderSpriteBatchEntry();
		void update();
		Texture *getTexture();
		void setParent(ShaderSpriteBatch *spriteBatch);
		void setX(float x);
		void setY(float y);
		void setWidth(float width);
		void setHeight(float height);
		inline uint8_t getChanges() {return this->changes;};
		inline void removeChange(uint8_t change) {this->changes &= ~change;};

	};

}

#endif
