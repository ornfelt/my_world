#include "ShaderSpriteBatchEntry.h"
#include "./ShaderSpriteUpdate.h"
#include "./ShaderSpriteBatch.h"

namespace librender
{

	ShaderSpriteBatchEntry::ShaderSpriteBatchEntry()
	: ShaderSpriteEntry()
	, parent(nullptr)
	, changes(0)
	{
	}

	ShaderSpriteBatchEntry::~ShaderSpriteBatchEntry()
	{
		if (this->parent)
			this->parent->removeEntry(this);
	}

	void ShaderSpriteBatchEntry::update()
	{
		uint8_t oldUpdates = this->updatesRequired;
		ShaderSpriteEntry::update();
		this->changes = this->updatesRequired | oldUpdates;
		this->parent->addChanges(this->changes);
	}

	void ShaderSpriteBatchEntry::setParent(ShaderSpriteBatch *spriteBatch)
	{
		if (this->parent && (!spriteBatch || this->parent->getTexture() != spriteBatch->getTexture()))
		{
			this->updatesRequired |= SHADER_SPRITE_UPDATE_VERTEXES;
			this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
			this->updatesRequired |= SHADER_SPRITE_UPDATE_COLORS;
		}
		this->parent = spriteBatch;
	}

	Texture *ShaderSpriteBatchEntry::getTexture()
	{
		if (this->parent)
			return this->parent->getTexture();
		return nullptr;
	}

	void ShaderSpriteBatchEntry::setX(float x)
	{
		if (this->pos.x == x)
			return;
		float delta = x - this->pos.x;
		for (uint32_t i = 0; i < this->verticesNumber; ++i)
			this->vertexes[i].x += delta;
		this->pos.x = x;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_VERTEXES;
	}

	void ShaderSpriteBatchEntry::setY(float y)
	{
		if (this->pos.y == y)
			return;
		float delta = y - this->pos.y;
		for (uint32_t i = 0; i < this->verticesNumber; ++i)
			this->vertexes[i].y += delta;
		this->pos.y = y;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_VERTEXES;
	}
	
	void ShaderSpriteBatchEntry::setWidth(float width)
	{
		float delta = width - (this->vertexes[1].x - this->pos.x);
		if (!delta)
			return;
		this->vertexes[1].x = width + this->pos.x;
		this->vertexes[2].x = width + this->pos.x;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_VERTEXES;
	}

	void ShaderSpriteBatchEntry::setHeight(float height)
	{
		float delta = height - (this->vertexes[3].y - this->pos.y);
		if (!delta)
			return;
		this->vertexes[2].y = height + this->pos.y;
		this->vertexes[3].y = height + this->pos.y;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_VERTEXES;
	}

}
