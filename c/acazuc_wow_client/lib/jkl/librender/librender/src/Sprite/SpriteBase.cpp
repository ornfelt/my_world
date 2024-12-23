#include "SpriteBase.h"
#include "../DrawableBuffers.h"
#include <cstring>

namespace librender
{

	SpriteBase::SpriteBase()
	: size(0)
	{
		this->verticesNumber = 4;
		this->texCoords.resize(4);
		this->texCoords[0] = Vec2(0, 0);
		this->texCoords[1] = Vec2(1, 0);
		this->texCoords[2] = Vec2(1, 1);
		this->texCoords[3] = Vec2(0, 1);
		this->colors.resize(4, Vec4(1));
		this->positions.resize(4, Vec2(0));
		this->indicesNumber = 6;
		this->indices.resize(6);
		this->indices[0] = 0;
		this->indices[1] = 1;
		this->indices[2] = 2;
		this->indices[3] = 2;
		this->indices[4] = 3;
		this->indices[5] = 0;
	}

	void SpriteBase::updatePositions()
	{
		this->positions[0] = Vec2(0, 0);
		this->positions[1] = Vec2(this->size.x, 0);
		this->positions[2] = this->size;
		this->positions[3] = Vec2(0, this->size.y);
	}

	void SpriteBase::setColor(Color color)
	{
		this->colors[0] = color;
		this->colors[1] = color;
		this->colors[2] = color;
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void SpriteBase::setTopColor(Color color)
	{
		this->colors[0] = color;
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void SpriteBase::setBotColor(Color color)
	{
		this->colors[2] = color;
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void SpriteBase::setLeftColor(Color color)
	{
		this->colors[0] = color;
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}
	
	void SpriteBase::setRightColor(Color color)
	{
		this->colors[1] = color;
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void SpriteBase::setTopLeftColor(Color color)
	{
		this->colors[0] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void SpriteBase::setTopRightColor(Color color)
	{
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void SpriteBase::setBotLeftColor(Color color)
	{
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void SpriteBase::setBotRightColor(Color color)
	{
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void SpriteBase::setTexTopLeft(Vec2 pos)
	{
		this->texCoords[0] = pos;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS);
	}

	Vec2 SpriteBase::getTexTopLeft()
	{
		return this->texCoords[0];
	}

	void SpriteBase::setTexTopRight(Vec2 pos)
	{
		this->texCoords[1] = pos;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS);
	}

	Vec2 SpriteBase::getTexTopRight()
	{
		return this->texCoords[1];
	}

	void SpriteBase::setTexBottomLeft(Vec2 pos)
	{
		this->texCoords[3] = pos;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS);
	}

	Vec2 SpriteBase::getTexBottomLeft()
	{
		return this->texCoords[3];
	}

	void SpriteBase::setTexBottomRight(Vec2 pos)
	{
		this->texCoords[2] = pos;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS);
	}

	Vec2 SpriteBase::getTexBottomRight()
	{
		return this->texCoords[2];
	}

	void SpriteBase::setTex(Vec2 topLeft, Vec2 topRight, Vec2 bottomLeft, Vec2 bottomRight)
	{
		this->texCoords[0] = topLeft;
		this->texCoords[1] = topRight;
		this->texCoords[2] = bottomRight;
		this->texCoords[3] = bottomLeft;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS);
	}

	void SpriteBase::setTexX(float texX)
	{
		float delta = texX - this->texCoords[0].x;
		if (!delta)
			return;
		this->texCoords[0].x = texX;
		this->texCoords[3].x = texX;
		this->texCoords[1].x += delta;
		this->texCoords[2].x += delta;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS);
	}

	void SpriteBase::setTexY(float texY)
	{
		float delta = texY - this->texCoords[0].y;
		if (!delta)
			return;
		this->texCoords[0].y = texY;
		this->texCoords[1].y = texY;
		this->texCoords[2].y += delta;
		this->texCoords[3].y += delta;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS);
	}

	void SpriteBase::setTexPos(float texX, float texY)
	{
		setTexX(texX);
		setTexY(texY);
	}

	void SpriteBase::setTexWidth(float texWidth)
	{
		float delta = texWidth - (this->texCoords[1].x - this->texCoords[0].x);
		if (!delta)
			return;
		this->texCoords[1].x = texWidth + this->texCoords[0].x;
		this->texCoords[2].x = texWidth + this->texCoords[3].x;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS);
	}

	void SpriteBase::setTexHeight(float texHeight)
	{
		float delta = texHeight - (this->texCoords[3].y - this->texCoords[0].y);
		if (!delta)
			return;
		this->texCoords[2].y = texHeight + this->texCoords[1].y;
		this->texCoords[3].y = texHeight + this->texCoords[0].y;
		requireUpdates(DRAWABLE_BUFFER_TEX_COORDS);
	}

	void SpriteBase::setTexSize(float texWidth, float texHeight)
	{
		setTexWidth(texWidth);
		setTexHeight(texHeight);
	}

	void SpriteBase::setWidth(float width)
	{
		if (this->size.x == width)
			return;
		this->size.x = width;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	float SpriteBase::getWidth() const
	{
		return this->size.x * this->scale.x;
	}

	void SpriteBase::setHeight(float height)
	{
		if (this->size.y == height)
			return;
		this->size.y = height;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	float SpriteBase::getHeight() const
	{
		return this->size.y * this->scale.y;
	}

	void SpriteBase::setSize(Vec2 size)
	{
		if (this->size == size)
			return;
		this->size = size;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	int32_t SpriteBase::getTextureWidth() const
	{
		if (!getTexture())
			return 0;
		return getTexture()->getWidth();
	}

	int32_t SpriteBase::getTextureHeight() const
	{
		if (!getTexture())
			return 0;
		return getTexture()->getHeight();
	}

}
