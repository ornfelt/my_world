#ifndef SPRITE_TESSELLATOR_H
# define SPRITE_TESSELLATOR_H

# include "../DrawableTessellator.h"
# include "../Texture/Texture.h"

namespace librender
{

	class SpriteTessellator : public DrawableTessellator
	{

	private:
		Texture *texture;

	public:
		SpriteTessellator();
		void draw();
		void addQuad(Vec2 pos, Vec2 size, Vec2 texOrg, Vec2 texSize, Vec4 color);
		inline void setTexture(Texture *texture) {this->texture = texture;};
		inline Texture *getTexture() {return this->texture;};

	};

}

#endif
