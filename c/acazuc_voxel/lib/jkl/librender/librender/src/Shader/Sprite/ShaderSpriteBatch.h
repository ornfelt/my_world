#ifndef LIBRENDER_SHADER_SPRITE_BATCH_H
# define LIBRENDER_SHADER_SPRITE_BATCH_H

# include "./ShaderSpriteBatchEntry.h"
# include "./ShaderSpriteProgram.h"
# include <vector>

namespace librender
{

	class ShaderSpriteBatch
	{

	private:
		std::vector<ShaderSpriteBatchEntry*> entries;
		ShaderSpriteProgram program;
		VertexBuffer texCoordsBuffer;
		VertexBuffer vertexesBuffer;
		VertexBuffer indicesBuffer;
		VertexBuffer colorsBuffer;
		std::vector<Vec2> texCoords;
		std::vector<Vec2> vertexes;
		std::vector<Vec4> colors;
		Texture *texture;
		Vec2 pos;
		uint32_t verticesNumber;
		uint8_t changes;
		bool mustResize;
		void updateVerticesNumber();
		void updateTexCoords();
		void updateVertexes();
		void updateIndices();
		void updateColors();
		void resize();

	public:
		ShaderSpriteBatch();
		~ShaderSpriteBatch();
		void draw(Mat4 &viewProj);
		void addEntry(ShaderSpriteBatchEntry *entry);
		void removeEntry(ShaderSpriteBatchEntry *entry);
		inline void setProgram(ShaderSpriteProgram program) {this->program = program;};
		inline ShaderSpriteProgram &getProgram() {return this->program;};
		inline std::vector<ShaderSpriteBatchEntry*> &getEntries() {return this->entries;};
		void setTexture(Texture *texture);
		inline Texture *getTexture() {return this->texture;};
		inline void addChanges(uint8_t changes) {this->changes |= changes;};
		inline void setPos(float x, float y) {setX(x);setY(y);};
		inline void setX(float x) {this->pos.x = x;};
		inline void setY(float y) {this->pos.y = y;};
		inline void setMustResize(bool mustResize) {this->mustResize = mustResize;};

	};

}

#endif
