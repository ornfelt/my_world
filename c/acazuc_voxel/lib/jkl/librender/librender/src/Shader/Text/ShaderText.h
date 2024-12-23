#ifndef LIBRENDER_SHADER_TEXT_H
# define LIBRENDER_SHADER_TEXT_H

# include "./ShaderTextProgram.h"
# include "./ShaderTextEntry.h"
# include "../../Font/Font.h"

namespace librender
{

	class ShaderText : public ShaderTextEntry
	{

	protected:
		ShaderTextProgram program;
		VertexBuffer texCoordsBuffer;
		VertexBuffer vertexesBuffer;
		VertexBuffer indicesBuffer;
		VertexBuffer colorsBuffer;
		Font *font;
		uint32_t oldVerticesNumber;

	public:
		ShaderText();
		~ShaderText();
		void draw(Mat4 &viewProj);
		inline void setProgram(ShaderTextProgram program) {this->program = program;};
		inline ShaderTextProgram &getProgram() {return this->program;};
		void setFont(Font *font);
		inline Font *getFont() {return this->font;};

	};

}

#endif
