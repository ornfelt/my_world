#include "ShaderSpriteTessellator.h"

namespace librender
{

	ShaderSpriteTessellator::ShaderSpriteTessellator()
	: texture(nullptr)
	, changed(false)
	{
	}

	void ShaderSpriteTessellator::clear()
	{
		std::vector<Vec2> emptyTexCoords;
		std::vector<Vec2> emptyVertexes;
		std::vector<Vec4> emptyColors;
		this->texCoords.swap(emptyTexCoords);
		this->vertexes.swap(emptyVertexes);
		this->colors.swap(emptyColors);
	}

	void ShaderSpriteTessellator::draw(Mat4 &viewProj)
	{
		if (!this->texture)
			return;
		if (this->changed)
		{
			this->texCoordsBuffer.setData(GL_ARRAY_BUFFER, this->texCoords.data(), sizeof(Vec2) * this->texCoords.size(), GL_DYNAMIC_DRAW);
			this->vertexesBuffer.setData(GL_ARRAY_BUFFER, this->vertexes.data(), sizeof(Vec2) * this->vertexes.size(), GL_DYNAMIC_DRAW);
			this->colorsBuffer.setData(GL_ARRAY_BUFFER, this->colors.data(), sizeof(Vec4) * this->colors.size(), GL_DYNAMIC_DRAW);
			this->indicesBuffer.setData(GL_ELEMENT_ARRAY_BUFFER, this->indices.data(), sizeof(GLuint) * this->indices.size(), GL_DYNAMIC_DRAW);
		}
		this->texture->bind();
		this->program->use();
		this->texCoordsLocation->setVertexBuffer(this->texCoordsBuffer, 2, GL_FLOAT);
		this->vertexesLocation->setVertexBuffer(this->vertexesBuffer, 2, GL_FLOAT);
		this->colorsLocation->setVertexBuffer(this->colorsBuffer, 4, GL_FLOAT);
		this->indicesBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
		Mat4 model(1);
		Mat4 mvp(viewProj * model);
		this->mvpLocation->setMat4f(mvp);
		glDrawArrays(GL_TRIANGLES, 0, this->indices.size());
	}

	void ShaderSpriteTessellator::add(Vec2 vertex, Vec2 texCoords, Vec4 color)
	{
		this->changed = true;
		this->texCoords.push_back(texCoords);
		this->vertexes.push_back(vertex);
		this->colors.push_back(color);
	}

	void ShaderSpriteTessellator::addQuad(Vec2 pos, Vec2 size, Vec2 texPos, Vec2 texSize, Vec4 color)
	{
		Vec2 org(pos);
		Vec2 dst(pos + size);
		Vec2 texOrg(texPos);
		Vec2 texDst(texPos + texSize);
		GLuint indice = this->texCoords.size();
		add(Vec2(org.x, org.y), Vec2(texOrg.x, texOrg.y), color);
		add(Vec2(dst.x, org.y), Vec2(texDst.x, texOrg.y), color);
		add(Vec2(dst.x, dst.y), Vec2(texDst.x, texDst.y), color);
		add(Vec2(org.x, dst.y), Vec2(texOrg.x, texDst.y), color);
		this->indices.push_back(indice + 0);
		this->indices.push_back(indice + 1);
		this->indices.push_back(indice + 3);
		this->indices.push_back(indice + 2);
		this->indices.push_back(indice + 3);
		this->indices.push_back(indice + 1);
	}

}
