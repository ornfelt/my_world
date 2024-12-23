#include "Rectangle.h"
#include "../DrawableBuffers.h"

namespace librender
{

	RectangleBase::RectangleBase()
	: size(0)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
		this->verticesNumber = 4;
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

	void RectangleBase::updatePositions()
	{
		this->positions[0] = Vec2(0, 0);
		this->positions[1] = Vec2(this->size.x, 0);
		this->positions[2] = this->size;
		this->positions[3] = Vec2(0, this->size.y);
	}

	void RectangleBase::setColor(Color color)
	{
		this->colors[0] = color;
		this->colors[1] = color;
		this->colors[2] = color;
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBase::setTopColor(Color color)
	{
		this->colors[0] = color;
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBase::setBotColor(Color color)
	{
		this->colors[2] = color;
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBase::setLeftColor(Color color)
	{
		this->colors[0] = color;
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}
	
	void RectangleBase::setRightColor(Color color)
	{
		this->colors[1] = color;
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBase::setTopLeftColor(Color color)
	{
		this->colors[0] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBase::setTopRightColor(Color color)
	{
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBase::setBotLeftColor(Color color)
	{
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBase::setBotRightColor(Color color)
	{
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBase::setWidth(float width)
	{
		if (this->size.x == width)
			return;
		this->size.x = width;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	float RectangleBase::getWidth()
	{
		return this->size.x * this->scale.x;
	}

	void RectangleBase::setHeight(float height)
	{
		if (this->size.y == height)
			return;
		this->size.y = height;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	float RectangleBase::getHeight()
	{
		return this->size.y * this->scale.y;
	}

	void RectangleBase::setSize(Vec2 size)
	{
		if (this->size == size)
			return;
		this->size = size;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	Rectangle::Rectangle()
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	RectangleBatch::RectangleBatch()
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	RectangleTessellator::RectangleTessellator()
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	void RectangleTessellator::addRectangle(Vec2 pos, Vec2 size, Color colorTopLeft, Color colorTopRight, Color colorBotRight, Color colorBotLeft)
	{
		Vec2 dst(pos + size);
		size_t base = this->positions.size();
		this->positions.push_back(Vec2(pos.x, pos.y));
		this->positions.push_back(Vec2(dst.x, pos.y));
		this->positions.push_back(Vec2(dst.x, dst.y));
		this->positions.push_back(Vec2(pos.x, dst.y));
		this->indices.push_back(base + 0);
		this->indices.push_back(base + 1);
		this->indices.push_back(base + 2);
		this->indices.push_back(base + 2);
		this->indices.push_back(base + 3);
		this->indices.push_back(base + 0);
		this->colors.push_back(colorTopLeft);
		this->colors.push_back(colorTopRight);
		this->colors.push_back(colorBotRight);
		this->colors.push_back(colorBotLeft);
	}

	void RectangleTessellator::addRectangle(Vec2 pos, Vec2 size, Color color)
	{
		addRectangle(pos, size, color, color, color, color);
	}

}
