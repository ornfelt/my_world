#include "Circle.h"
#include "../DrawableBuffers.h"
#include "../GL.h"

namespace librender
{

	CircleBase::CircleBase()
	: size(0)
	, begin(0)
	, end(M_PI * 2)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
		std::fill(std::begin(this->colorPoints), std::end(this->colorPoints), Vec4(1));
	}

	void CircleBase::updatePositions()
	{
		if (!this->verticesNumber)
			return;
		float tmp = (this->end - this->begin) / (this->verticesNumber - 1);
		Vec2 factor(this->size * .5f);
		for (size_t i = 0; i < this->verticesNumber; ++i)
		{
			float pos = this->begin + i * tmp;
			this->positions[i] = Vec2(cos(pos), sin(pos)) * factor;
		}
	}

	void CircleBase::updateIndices()
	{
		if (!this->indicesNumber)
			return;
		size_t idx = 0;
		for (size_t i = 0; i < this->indicesNumber;)
		{
			this->indices[i++] = idx++;
			this->indices[i++] = idx;
		}
	}

	void CircleBase::updateColors()
	{
		if (!this->verticesNumber)
			return;
		Vec4 delta = this->colorPoints[1] - this->colorPoints[0];
		for (size_t i = 0; i < this->verticesNumber; ++i)
		{
			float pos = i / static_cast<float>(this->verticesNumber - 1);
			this->colors[i] = this->colorPoints[0] + pos * delta;
		}
	}

	void CircleBase::setPoints(size_t count)
	{
		if (count < 2)
			count = 0;
		resize(count, (count - 1) * 2);
	}

	void CircleBase::setColor(Color color)
	{
		this->colorPoints[0] = color;
		this->colorPoints[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void CircleBase::setBeginColor(Color color)
	{
		this->colorPoints[0] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void CircleBase::setEndColor(Color color)
	{
		this->colorPoints[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void CircleBase::setBegin(float begin)
	{
		if (this->begin == begin)
			return;
		this->begin = begin;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_COLORS);
	}

	void CircleBase::setEnd(float end)
	{
		if (this->end == end)
			return;
		this->end = end;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_COLORS);
	}

	void CircleBase::setWidth(float width)
	{
		if (this->size.x == width)
			return;
		this->size.x = width;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void CircleBase::setHeight(float height)
	{
		if (this->size.y == height)
			return;
		this->size.y = height;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void CircleBase::setSize(Vec2 size)
	{
		if (this->size == size)
			return;
		this->size = size;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	Circle::Circle()
	: Shape(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	CircleBatch::CircleBatch()
	: ShapeBatch(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	CircleTessellator::CircleTessellator()
	: ShapeTessellator(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	void CircleTessellator::addCircle(Vec2 pos, Vec2 size, float begin, float end, Color beginColor, Color endColor, size_t points)
	{
		if (points == 0)
			points = std::max(10., M_PI * length(size) / 4);
		if (points < 3)
			return;
		size_t base = this->positions.size();
		{
			float tmp = (end - begin) / (points - 1);
			Vec4 delta(endColor - beginColor);
			Vec2 factor(size * .5f);
			for (size_t i = 0; i < points; ++i)
			{
				float idx = begin + i * tmp;
				Vec2 loc(cos(idx), sin(idx));
				this->positions.push_back(pos + loc * factor);
				float per = i / static_cast<float>(points - 1);
				this->colors.push_back(beginColor + per * delta);
			}
		}
		{
			size_t idx = base;
			for (size_t i = 0; i < (points - 1); i++)
			{
				this->indices.push_back(idx++);
				this->indices.push_back(idx);
			}
		}
	}

	void CircleTessellator::addCircle(Vec2 pos, Vec2 size, float begin, float end, Color color, size_t points)
	{
		addCircle(pos, size, begin, end, color, color, points);
	}

	void CircleTessellator::addCircle(Vec2 pos, Vec2 size, Color beginColor, Color endColor, size_t points)
	{
		addCircle(pos, size, 0, M_PI * 2, beginColor, endColor, points);
	}

	void CircleTessellator::addCircle(Vec2 pos, Vec2 size, Color color, size_t points)
	{
		addCircle(pos, size, 0, M_PI * 2, color, color, points);
	}

}
