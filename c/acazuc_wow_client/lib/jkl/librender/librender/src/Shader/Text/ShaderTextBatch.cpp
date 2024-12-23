#include "ShaderTextBatch.h"
#include "./ShaderTextUpdate.h"
#include "../../GL.h"
#include <cstring>

namespace librender
{

	ShaderTextBatch::ShaderTextBatch()
	: font(nullptr)
	, pos(0)
	, verticesNumber(0)
	, changes(0)
	, mustResize(true)
	{
	}

	ShaderTextBatch::~ShaderTextBatch()
	{
		for (uint32_t i = 0; i < this->entries.size(); ++i)
			this->entries[i]->setParent(nullptr);
	}

	void ShaderTextBatch::updateVerticesNumber()
	{
		this->verticesNumber = 0;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
			this->verticesNumber += this->entries[i]->getVerticesNumber();
	}

	void ShaderTextBatch::updateTexCoords()
	{
		uint32_t count = 0;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			ShaderTextBatchEntry *entry = this->entries[i];
			if (this->mustResize || entry->getChanges() & SHADER_TEXT_UPDATE_TEX_COORDS)
			{
				std::memcpy(&this->texCoords[count], entry->getTexCoords().data(), entry->getVerticesNumber() * sizeof(*this->texCoords.data()));
				entry->removeChanges(SHADER_TEXT_UPDATE_TEX_COORDS);
			}
			count += entry->getVerticesNumber();
		}
		this->texCoordsBuffer.setData(GL_ARRAY_BUFFER, this->texCoords.data(), count * sizeof(*this->texCoords.data()), GL_DYNAMIC_DRAW);
	}

	void ShaderTextBatch::updateVertexes()
	{
		uint32_t count = 0;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			ShaderTextBatchEntry *entry = this->entries[i];
			if (this->mustResize || entry->getChanges() & SHADER_TEXT_UPDATE_VERTEXES)
			{
				std::memcpy(&this->vertexes[count], entry->getVertexes().data(), entry->getVerticesNumber() * sizeof(*this->vertexes.data()));
				entry->removeChanges(SHADER_TEXT_UPDATE_VERTEXES);
			}
			count += entry->getVerticesNumber();
		}
		this->vertexesBuffer.setData(GL_ARRAY_BUFFER, this->vertexes.data(), count * sizeof(*this->vertexes.data()), GL_DYNAMIC_DRAW);
	}

	void ShaderTextBatch::updateIndices()
	{
		std::vector<GLint> indices(this->verticesNumber / 4 * 6);
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

	void ShaderTextBatch::updateColors()
	{
		uint32_t count = 0;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			ShaderTextBatchEntry *entry = this->entries[i];
			if (this->mustResize || entry->getChanges() & SHADER_TEXT_UPDATE_COLORS)
			{
				std::memcpy(&this->colors[count], entry->getColors().data(), entry->getVerticesNumber() * sizeof(*this->colors.data()));
				entry->removeChanges(SHADER_TEXT_UPDATE_COLORS);
			}
			count += entry->getVerticesNumber();
		}
		this->colorsBuffer.setData(GL_ARRAY_BUFFER, this->colors.data(), count * sizeof(*this->colors.data()), GL_DYNAMIC_DRAW);
	}

	void ShaderTextBatch::resize()
	{
		updateVerticesNumber();
		if (!this->verticesNumber)
			return;
		this->texCoords.resize(this->verticesNumber);
		this->vertexes.resize(this->verticesNumber);
		this->colors.resize(this->verticesNumber);
	}

	void ShaderTextBatch::draw(Mat4 &viewProj)
	{
		if (!this->font || !this->program.texCoordsLocation || !this->program.vertexesLocation || !this->program.colorsLocation || !this->program.mvpLocation)
			return;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
			this->entries[i]->update();
		if (this->mustResize)
			resize();
		if (!this->verticesNumber)
			return;
		if (this->mustResize)
		{
			this->changes = SHADER_TEXT_UPDATE_TEX_COORDS | SHADER_TEXT_UPDATE_VERTEXES | SHADER_TEXT_UPDATE_COLORS;
			updateIndices();
		}
		if (this->changes & SHADER_TEXT_UPDATE_TEX_COORDS)
			updateTexCoords();
		if (this->changes & SHADER_TEXT_UPDATE_VERTEXES)
			updateVertexes();
		if (this->changes & SHADER_TEXT_UPDATE_COLORS)
			updateColors();
		if (this->mustResize)
			this->mustResize = false;
		this->changes = 0;
		this->font->bind();
		if (this->program.program)
			this->program.program->use();
		this->program.texCoordsLocation->setVertexBuffer(this->texCoordsBuffer, 2, GL_FLOAT);
		this->program.vertexesLocation->setVertexBuffer(this->vertexesBuffer, 2, GL_FLOAT);
		this->program.colorsLocation->setVertexBuffer(this->colorsBuffer, 4, GL_FLOAT);
		Mat4 model(Mat4::translate(Mat4(1), Vec3(this->pos, 0)));
		Mat4 mvp(viewProj * model);
		this->program.mvpLocation->setMat4f(mvp);
		glDrawElements(GL_TRIANGLES, this->verticesNumber / 4 * 6, GL_UNSIGNED_INT, nullptr);
	}

	void ShaderTextBatch::addEntry(ShaderTextBatchEntry *entry)
	{
		entry->setParent(this);
		this->entries.push_back(entry);
		this->mustResize = true;
		this->changes = SHADER_TEXT_UPDATE_TEX_COORDS | SHADER_TEXT_UPDATE_VERTEXES | SHADER_TEXT_UPDATE_COLORS;
	}

	void ShaderTextBatch::removeEntry(ShaderTextBatchEntry *entry)
	{
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			if (this->entries[i] == entry)
			{
				this->entries.erase(this->entries.begin() + i);
				entry->setParent(nullptr);
				this->mustResize = true;
				this->changes = SHADER_TEXT_UPDATE_TEX_COORDS | SHADER_TEXT_UPDATE_VERTEXES | SHADER_TEXT_UPDATE_COLORS;
				return;
			}
		}
	}

	void ShaderTextBatch::clearEntries()
	{
		for (uint32_t i = 0; i < this->entries.size(); ++i)
			this->entries[i]->setParent(nullptr);
		this->entries.clear();
		this->mustResize = true;
		this->changes = SHADER_TEXT_UPDATE_TEX_COORDS | SHADER_TEXT_UPDATE_VERTEXES | SHADER_TEXT_UPDATE_COLORS;
	}

	void ShaderTextBatch::setFont(Font *font)
	{
		this->font = font;
		this->changes = SHADER_TEXT_UPDATE_TEX_COORDS | SHADER_TEXT_UPDATE_VERTEXES;
		for (uint32_t i = 0; i < this->entries.size(); ++i)
		{
			ShaderTextBatchEntry *entry = this->entries[i];
			entry->addChanges(SHADER_TEXT_UPDATE_TEX_COORDS | SHADER_TEXT_UPDATE_VERTEXES);
			entry->recalcWidth();
			entry->recalcHeight();
		}
	}

	int32_t ShaderTextBatch::getLineHeight()
	{
		if (!this->font)
			return 0;
		return this->font->getLineHeight();
	}

}
