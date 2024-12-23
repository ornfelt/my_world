#include "RectangleBorder.h"
#include "../DrawableBuffers.h"
#include "../GL.h"

namespace librender
{

	RectangleBorderBase::RectangleBorderBase()
	: size(0)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
		this->verticesNumber = 4;
		this->colors.resize(4, Vec4(1));
		this->positions.resize(4, Vec2(0));
		this->indicesNumber = 8;
		this->indices.resize(8);
		this->indices[0] = 0;
		this->indices[1] = 1;
		this->indices[2] = 1;
		this->indices[3] = 2;
		this->indices[4] = 2;
		this->indices[5] = 3;
		this->indices[6] = 3;
		this->indices[7] = 0;
	}

	void RectangleBorderBase::updatePositions()
	{
		this->positions[0] = Vec2(0, 0);
		this->positions[1] = Vec2(this->size.x, 0);
		this->positions[2] = this->size;
		this->positions[3] = Vec2(0, this->size.y);
	}

	void RectangleBorderBase::setColor(Color color)
	{
		this->colors[0] = color;
		this->colors[1] = color;
		this->colors[2] = color;
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBorderBase::setTopColor(Color color)
	{
		this->colors[0] = color;
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBorderBase::setBotColor(Color color)
	{
		this->colors[2] = color;
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBorderBase::setLeftColor(Color color)
	{
		this->colors[0] = color;
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}
	
	void RectangleBorderBase::setRightColor(Color color)
	{
		this->colors[1] = color;
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBorderBase::setTopLeftColor(Color color)
	{
		this->colors[0] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBorderBase::setTopRightColor(Color color)
	{
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBorderBase::setBotLeftColor(Color color)
	{
		this->colors[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBorderBase::setBotRightColor(Color color)
	{
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void RectangleBorderBase::setWidth(float width)
	{
		if (this->size.x == width)
			return;
		this->size.x = width;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	float RectangleBorderBase::getWidth()
	{
		return this->size.x * this->scale.x;
	}

	void RectangleBorderBase::setHeight(float height)
	{
		if (this->size.y == height)
			return;
		this->size.y = height;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	float RectangleBorderBase::getHeight()
	{
		return this->size.y * this->scale.y;
	}

	void RectangleBorderBase::setSize(Vec2 size)
	{
		if (this->size == size)
			return;
		this->size = size;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	RectangleBorder::RectangleBorder()
	: Shape(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	RectangleBorderBatch::RectangleBorderBatch()
	: ShapeBatch(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	RectangleBorderTessellator::RectangleBorderTessellator()
	: ShapeTessellator(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	void RectangleBorderTessellator::addRectangleBorder(Vec2 pos, Vec2 size, Color colorTopLeft, Color colorTopRight, Color colorBotRight, Color colorBotLeft)
	{
		Vec2 dst(pos + size);
		size_t base = this->positions.size();
		this->positions.push_back(Vec2(pos.x, pos.y));
		this->positions.push_back(Vec2(dst.x, pos.y));
		this->positions.push_back(Vec2(dst.x, dst.y));
		this->positions.push_back(Vec2(pos.x, dst.y));
		this->indices.push_back(base + 0);
		this->indices.push_back(base + 1);
		this->indices.push_back(base + 1);
		this->indices.push_back(base + 2);
		this->indices.push_back(base + 2);
		this->indices.push_back(base + 3);
		this->indices.push_back(base + 3);
		this->indices.push_back(base + 0);
		this->colors.push_back(colorTopLeft);
		this->colors.push_back(colorTopRight);
		this->colors.push_back(colorBotRight);
		this->colors.push_back(colorBotLeft);
	}

	void RectangleBorderTessellator::addRectangleBorder(Vec2 pos, Vec2 size, Color color)
	{
		addRectangleBorder(pos, size, color, color, color, color);
	}

}
