#include "ShaderText.h"
#include "./ShaderTextUpdate.h"
#include "../../GL.h"

namespace librender
{

	ShaderText::ShaderText()
	: font(nullptr)
	, oldVerticesNumber(0)
	{
	}

	ShaderText::~ShaderText()
	{
	}

	void ShaderText::draw(Mat4 &viewProj)
	{
		if (!this->font || !this->program.texCoordsLocation || !this->program.vertexesLocation || !this->program.colorsLocation || !this->program.mvpLocation)
			return;
		if (!this->verticesNumber)
			return;
		uint8_t changes = this->updatesRequired;
		update();
		if (changes & SHADER_TEXT_UPDATE_TEX_COORDS)
			this->texCoordsBuffer.setData(GL_ARRAY_BUFFER, this->texCoords.data(), sizeof(*this->texCoords.data()) * this->verticesNumber, GL_DYNAMIC_DRAW);
		if (changes & SHADER_TEXT_UPDATE_VERTEXES)
			this->vertexesBuffer.setData(GL_ARRAY_BUFFER, this->vertexes.data(), sizeof(*this->vertexes.data()) * this->verticesNumber, GL_DYNAMIC_DRAW);
		if (changes & SHADER_TEXT_UPDATE_COLORS)
			this->colorsBuffer.setData(GL_ARRAY_BUFFER, this->colors.data(), sizeof(*this->colors.data()) * this->verticesNumber, GL_DYNAMIC_DRAW);
		if (this->verticesNumber != this->oldVerticesNumber)
		{
			this->oldVerticesNumber = this->verticesNumber;
			GLuint *indices = new GLuint[this->verticesNumber / 4 * 6];
			GLuint pos = 0;
			GLuint current = 0;
			for (uint32_t i = 0; i < this->verticesNumber / 4; ++i)
			{
				indices[pos++] = current + 0;
				indices[pos++] = current + 3;
				indices[pos++] = current + 1;
				indices[pos++] = current + 2;
				indices[pos++] = current + 1;
				indices[pos++] = current + 3;
				current += 4;
			}
			this->indicesBuffer.setData(GL_ELEMENT_ARRAY_BUFFER, indices, sizeof(*indices) * this->verticesNumber / 4 * 6, GL_DYNAMIC_DRAW);
			delete[] (indices);
		}
		glActiveTexture(GL_TEXTURE0);
		this->font->bind();
		if (this->program.program)
			this->program.program->use();
		this->program.texCoordsLocation->setVertexBuffer(this->texCoordsBuffer, 2, GL_FLOAT);
		this->program.vertexesLocation->setVertexBuffer(this->vertexesBuffer, 2, GL_FLOAT);
		this->program.colorsLocation->setVertexBuffer(this->colorsBuffer, 4, GL_FLOAT);
		this->indicesBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
		Mat4 model(Mat4::translate(Mat4(1), Vec3(this->pos, 0)));
		model = Mat4::scale(model, Vec3(this->scale, 1));
		Mat4 mvp(viewProj * model);
		this->program.mvpLocation->setMat4f(mvp);
		glDrawElements(GL_TRIANGLES, this->verticesNumber / 4 * 6, GL_UNSIGNED_INT, nullptr);
	}

	void ShaderText::setFont(Font *font)
	{
		if (this->font == font)
			return;
		this->font = font;
		this->lineHeight = this->font->getLineHeight();
		this->updatesRequired |= SHADER_TEXT_UPDATE_VERTEXES;
		this->updatesRequired |= SHADER_TEXT_UPDATE_TEX_COORDS;
		recalcWidth();
		recalcHeight();
	}

}
