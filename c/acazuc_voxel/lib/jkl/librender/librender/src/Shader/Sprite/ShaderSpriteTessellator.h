#ifndef SHADER_SPRITE_TESSELLATOR_H
# define SHADER_SPRITE_TESSELLATOR_H

# include "../../Texture/Texture.h"
# include "../../Vec/Vec4.h"
# include "../Program.h"
# include "../../GL.h"
# include <vector>

namespace librender
{

	class ShaderSpriteTessellator
	{

	private:
		std::vector<GLuint> indices;
		std::vector<Vec2> texCoords;
		std::vector<Vec2> vertexes;
		std::vector<Vec4> colors;
		ProgramLocation *texCoordsLocation;
		ProgramLocation *vertexesLocation;
		ProgramLocation *colorsLocation;
		ProgramLocation *mvpLocation;
		VertexBuffer texCoordsBuffer;
		VertexBuffer vertexesBuffer;
		VertexBuffer indicesBuffer;
		VertexBuffer colorsBuffer;
		Texture *texture;
		Program *program;
		bool changed;

	public:
		ShaderSpriteTessellator();
		void clear();
		void draw(Mat4 &viewProj);
		void add(Vec2 vertex, Vec2 texCoords, Vec4 color);
		void addQuad(Vec2 pos, Vec2 size, Vec2 texOrg, Vec2 texSize, Vec4 color);
		inline void setTexCoordsLocation(ProgramLocation *location) {this->texCoordsLocation = location;};
		inline void setVertexesLocation(ProgramLocation *location) {this->vertexesLocation = location;};
		inline void setColorsLocation(ProgramLocation *location) {this->colorsLocation = location;};
		inline void setMvpLocation(ProgramLocation *location) {this->mvpLocation = location;};
		inline void setProgram(Program *program) {this->program = program;};
		inline void setTexture(Texture *texture) {this->texture = texture;};
		inline Texture *getTexture() {return this->texture;};

	};

}

#endif
