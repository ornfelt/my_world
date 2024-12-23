#include "ShaderTextBatchEntry.h"
#include "./ShaderTextUpdate.h"
#include "./ShaderTextBatch.h"

namespace librender
{

	ShaderTextBatchEntry::ShaderTextBatchEntry()
	: ShaderTextEntry()
	, parent(nullptr)
	, changes(0)
	{
	}

	ShaderTextBatchEntry::~ShaderTextBatchEntry()
	{
		if (this->parent)
			this->parent->removeEntry(this);
	}

	void ShaderTextBatchEntry::resize(uint32_t len)
	{
		if (this->parent)
			this->parent->setMustResize(true);
		ShaderTextEntry::resize(len);
	}

	void ShaderTextBatchEntry::requireUpdates(uint8_t updates)
	{
		ShaderTextEntry::requireUpdates(updates);
		addChanges(updates);
		if (this->parent)
			this->parent->addChanges(updates);
	}

	void ShaderTextBatchEntry::update()
	{
		ShaderTextEntry::update();
		if (this->changes & SHADER_TEXT_UPDATE_VERTEXES && (this->pos.x || this->pos.y || this->scale.x || this->scale.y))
		{
			for (uint32_t i = 0; i < this->verticesNumber; ++i)
			{
				this->vertexes[i] *= this->scale;
				this->vertexes[i] += this->pos;
			}
		}
	}

	void ShaderTextBatchEntry::setParent(ShaderTextBatch *textBatch)
	{
		if (this->parent && (!textBatch || this->parent->getFont() != textBatch->getFont()))
			requireUpdates(SHADER_TEXT_UPDATE_VERTEXES | SHADER_TEXT_UPDATE_TEX_COORDS | SHADER_TEXT_UPDATE_COLORS);
		this->parent = textBatch;
	}

	Font *ShaderTextBatchEntry::getFont()
	{
		if (this->parent)
			return this->parent->getFont();
		return nullptr;
	}

	void ShaderTextBatchEntry::setX(float x)
	{
		if (this->pos.x == x)
			return;
		float diffX = x - this->pos.x;
		for (uint32_t i = 0; i < this->verticesNumber; ++i)
			this->vertexes[i].x += diffX;
		this->pos.x = x;
	}

	void ShaderTextBatchEntry::setY(float y)
	{
		if (this->pos.y == y)
			return;
		float diffY = y - this->pos.y;
		for (uint32_t i = 0; i < this->verticesNumber; ++i)
			this->vertexes[i].y += diffY;
		this->pos.y = y;
	}

	void ShaderTextBatchEntry::setScaleX(float scaleX)
	{
		if (this->scale.x == scaleX)
			return;
		float ratioX = scaleX / this->scale.x;
		for (uint32_t i = 0; i < this->verticesNumber; ++i)
			this->vertexes[i].x *= ratioX;
		this->scale.x = scaleX;
	}

	void ShaderTextBatchEntry::setScaleY(float scaleY)
	{
		if (this->scale.y == scaleY)
			return;
		float ratioY = scaleY / this->scale.y;
		for (uint32_t i = 0; i < this->verticesNumber; ++i)
			this->vertexes[i].y *= ratioY;
		this->scale.y = scaleY;
	}

}
