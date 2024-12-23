#include "ShaderSpriteBatch.h"
#include "./ShaderSpriteUpdate.h"
#include "../../GL.h"
#include <cstring>

namespace librender
{

	ShaderSpriteBatch::ShaderSpriteBatch()
	: texture(nullptr)
	, pos(0)
	, verticesNumber(0)
	, changes(0)
	, mustResize(true)
	{
	}

	ShaderSpriteBatch::~ShaderSpriteBatch()
	{
		for (uint32_t i = 0; i < this->entries.size(); ++i)
			this->entries[i]->setParent(nullptr);
	}

	void ShaderSpriteBatch::updateVerticesNumber()
	{
		this->verticesNumber = 0;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
			this->verticesNumber += this->entries[i]->getVerticesNumber();
	}

	void ShaderSpriteBatch::updateTexCoords()
	{
		uint32_t count = 0;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			ShaderSpriteBatchEntry *entry = this->entries[i];
			if (this->mustResize || entry->getChanges() & SHADER_SPRITE_UPDATE_TEX_COORDS)
			{
				std::memcpy(&this->texCoords[count], entry->getTexCoords().data(), entry->getVerticesNumber() * sizeof(*this->texCoords.data()));
				entry->removeChange(SHADER_SPRITE_UPDATE_TEX_COORDS);
			}
			count += entry->getVerticesNumber();
		}
		this->texCoordsBuffer.setData(GL_ARRAY_BUFFER, this->texCoords.data(), count * sizeof(*this->texCoords.data()), GL_DYNAMIC_DRAW);
	}

	void ShaderSpriteBatch::updateVertexes()
	{
		uint32_t count = 0;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			ShaderSpriteBatchEntry *entry = this->entries[i];
			if (this->mustResize || entry->getChanges() & SHADER_SPRITE_UPDATE_VERTEXES)
			{
				std::memcpy(&this->vertexes[count], entry->getVertexes().data(), entry->getVerticesNumber() * sizeof(*this->vertexes.data()));
				entry->removeChange(SHADER_SPRITE_UPDATE_VERTEXES);
			}
			count += entry->getVerticesNumber();
		}
		this->vertexesBuffer.setData(GL_ARRAY_BUFFER, this->vertexes.data(), count * sizeof(*this->vertexes.data()), GL_DYNAMIC_DRAW);
	}

	void ShaderSpriteBatch::updateIndices()
	{
		std::vector<GLuint> indices(this->verticesNumber / 4 * 6);
		uint32_t count = 0;
		GLuint currentIndice = 0;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			indices[count++] = currentIndice + 0;
			indices[count++] = currentIndice + 3;
			indices[count++] = currentIndice + 1;
			indices[count++] = currentIndice + 2;
			indices[count++] = currentIndice + 1;
			indices[count++] = currentIndice + 3;
			currentIndice += this->entries[i]->getVerticesNumber();
		}
		this->indicesBuffer.setData(GL_ELEMENT_ARRAY_BUFFER, indices.data(), count * sizeof(*indices.data()), GL_DYNAMIC_DRAW);
	}

	void ShaderSpriteBatch::updateColors()
	{
		uint32_t count = 0;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			ShaderSpriteBatchEntry *entry = this->entries[i];
			if (this->mustResize || entry->getChanges() & SHADER_SPRITE_UPDATE_COLORS)
			{
				std::memcpy(&this->colors[count], entry->getColors().data(), entry->getVerticesNumber() * sizeof(*this->colors.data()));
				entry->removeChange(SHADER_SPRITE_UPDATE_COLORS);
			}
			count += entry->getVerticesNumber();
		}
		this->colorsBuffer.setData(GL_ARRAY_BUFFER, this->colors.data(), count * sizeof(*this->colors.data()), GL_DYNAMIC_DRAW);
	}

	void ShaderSpriteBatch::resize()
	{
		updateVerticesNumber();
		if (!this->verticesNumber)
			return;
		this->texCoords.resize(this->verticesNumber);
		this->vertexes.resize(this->verticesNumber);
		this->colors.resize(this->verticesNumber);
	}

	void ShaderSpriteBatch::draw(Mat4 &viewProj)
	{
		if (!this->program.texCoordsLocation || !this->program.vertexesLocation || !this->program.colorsLocation || !this->program.mvpLocation)
			return;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
			this->entries[i]->update();
		if (this->mustResize)
			resize();
		if (!this->verticesNumber)
			return;
		if (this->mustResize)
		{
			this->changes = SHADER_SPRITE_UPDATE_TEX_COORDS | SHADER_SPRITE_UPDATE_VERTEXES | SHADER_SPRITE_UPDATE_COLORS;
			updateIndices();
		}
		if (this->changes & SHADER_SPRITE_UPDATE_TEX_COORDS)
			updateTexCoords();
		if (this->changes & SHADER_SPRITE_UPDATE_VERTEXES)
			updateVertexes();
		if (this->changes & SHADER_SPRITE_UPDATE_COLORS)
			updateColors();
		if (this->mustResize)
			this->mustResize = false;
		this->changes = 0;
		if (this->texture)
			this->texture->bind();
		else
			glBindTexture(GL_TEXTURE_2D, 0);
		if (this->program.program)
			this->program.program->use();
		this->program.texCoordsLocation->setVertexBuffer(this->texCoordsBuffer, 2, GL_FLOAT);
		this->program.vertexesLocation->setVertexBuffer(this->vertexesBuffer, 2, GL_FLOAT);
		this->program.colorsLocation->setVertexBuffer(this->colorsBuffer, 4, GL_FLOAT);
		this->indicesBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
		Mat4 model(Mat4::translate(Mat4(1), Vec3(this->pos, 0)));
		Mat4 mvp(viewProj * model);
		this->program.mvpLocation->setMat4f(mvp);
		glDrawElements(GL_TRIANGLES, this->verticesNumber * 6 / 4, GL_UNSIGNED_INT, nullptr);
	}

	void ShaderSpriteBatch::addEntry(ShaderSpriteBatchEntry *entry)
	{
		entry->setParent(this);
		this->entries.push_back(entry);
	}

	void ShaderSpriteBatch::removeEntry(ShaderSpriteBatchEntry *entry)
	{
		entry->setParent(nullptr);
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			if (this->entries[i] == entry)
			{
				this->entries.erase(this->entries.begin() + i);
				return;
			}
		}
	}

	void ShaderSpriteBatch::setTexture(Texture *texture)
	{
		this->texture = texture;
		this->changes = SHADER_SPRITE_UPDATE_TEX_COORDS | SHADER_SPRITE_UPDATE_VERTEXES | SHADER_SPRITE_UPDATE_COLORS;
	}

}
