#ifndef LIBRENDER_SHADER_SPRITE_H
# define LIBRENDER_SHADER_SPRITE_H

# include "./ShaderSpriteProgram.h"
# include "./ShaderSpriteEntry.h"

namespace librender
{

	class ShaderSprite : public ShaderSpriteEntry
	{

	protected:
		ShaderSpriteProgram program;
		VertexBuffer texCoordsBuffer;
		VertexBuffer vertexesBuffer;
		VertexBuffer indicesBuffer;
		VertexBuffer colorsBuffer;
		Texture *texture;

	public:
		ShaderSprite();
		~ShaderSprite();
		void draw(Mat4 &viewProj);
		inline void setProgram(ShaderSpriteProgram program) {this->program = program;};
		inline ShaderSpriteProgram &getProgram() {return this->program;};
		void setTexture(Texture *texture);
		inline Texture *getTexture() {return this->texture;};

	};

}

#endif
