#include "Disk.h"
#include "../DrawableBuffers.h"

namespace librender
{

	DiskBase::DiskBase()
	: size(0)
	, begin(0)
	, end(M_PI * 2)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
		std::fill(std::begin(this->colorPoints), std::end(this->colorPoints), Vec4(1));
	}

	void DiskBase::updatePositions()
	{
		if (!this->verticesNumber)
			return;
		float tmp = (this->end - this->begin) / (this->verticesNumber - 2);
		this->positions[0] = Vec2(0, 0);
		Vec2 factor(this->size * .5f);
		for (size_t i = 1; i < this->verticesNumber; ++i)
		{
			float pos = this->begin + (i - 1) * tmp;
			this->positions[i] = Vec2(cos(pos), sin(pos)) * factor;
		}
	}

	void DiskBase::updateIndices()
	{
		if (!this->indicesNumber)
			return;
		size_t idx = 1;
		size_t i = 0;
		while (i < this->indicesNumber)
		{
			this->indices[i++] = 0;
			this->indices[i++] = idx++;
			this->indices[i++] = idx;
		}
	}

	void DiskBase::updateColors()
	{
		if (!this->verticesNumber)
			return;
		float tmp = (this->end - this->begin) / (this->verticesNumber - 2);
		Vec4 dx = (this->colorPoints[1] - this->colorPoints[0]);
		Vec4 dy = (this->colorPoints[2] - this->colorPoints[3]);
		this->colors[0] = (this->colorPoints[0] + this->colorPoints[1] + this->colorPoints[2] + this->colorPoints[3]) * .25f;
		for (size_t i = 1; i < this->verticesNumber; ++i)
		{
			float pos = this->begin + (i - 1) * tmp;
			Vec2 loc(cos(pos), sin(pos));
			loc = loc * .5f + .5f;
			Vec4 top(loc.x * dx + this->colorPoints[0]);
			Vec4 bot(loc.x * dy + this->colorPoints[3]);
			this->colors[i] = loc.y * (bot - top) + top;
		}
	}

	void DiskBase::setPoints(size_t count)
	{
		if (count < 3)
			count = 0;
		resize(1 + count, (count - 1) * 3);
	}

	void DiskBase::setColor(Color color)
	{
		this->colorPoints[0] = color;
		this->colorPoints[1] = color;
		this->colorPoints[2] = color;
		this->colorPoints[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setTopColor(Color color)
	{
		this->colorPoints[0] = color;
		this->colorPoints[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setBotColor(Color color)
	{
		this->colorPoints[2] = color;
		this->colorPoints[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setLeftColor(Color color)
	{
		this->colorPoints[0] = color;
		this->colorPoints[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}
	
	void DiskBase::setRightColor(Color color)
	{
		this->colorPoints[1] = color;
		this->colorPoints[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setTopLeftColor(Color color)
	{
		this->colorPoints[0] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setTopRightColor(Color color)
	{
		this->colorPoints[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setBotLeftColor(Color color)
	{
		this->colorPoints[3] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setBotRightColor(Color color)
	{
		this->colorPoints[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setBegin(float begin)
	{
		if (this->begin == begin)
			return;
		this->begin = begin;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setEnd(float end)
	{
		if (this->end == end)
			return;
		this->end = end;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_TEX_COORDS | DRAWABLE_BUFFER_COLORS);
	}

	void DiskBase::setWidth(float width)
	{
		if (this->size.x == width)
			return;
		this->size.x = width;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void DiskBase::setHeight(float height)
	{
		if (this->size.y == height)
			return;
		this->size.y = height;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void DiskBase::setSize(Vec2 size)
	{
		if (this->size == size)
			return;
		this->size = size;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	Disk::Disk()
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	DiskBatch::DiskBatch()
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	DiskTessellator::DiskTessellator()
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	void DiskTessellator::addDisk(Vec2 pos, Vec2 size, float begin, float end, Color colorTopLeft, Color colorTopRight, Color colorBotRight, Color colorBotLeft, size_t points)
	{
		if (points == 0)
			points = std::max(10., M_PI * length(size) / 4);
		if (points < 3)
			return;
		size_t base = this->positions.size();
		{
			float tmp = (end - begin) / (points - 1);
			Vec4 dx = (colorTopRight - colorTopLeft);
			Vec4 dy = (colorBotRight - colorBotLeft);
			this->positions.push_back(pos);
			this->colors.push_back((colorTopLeft + colorTopRight + colorBotRight + colorBotLeft) * .25f);
			Vec2 factor(size * .5f);
			for (size_t i = 0; i < points; ++i)
			{
				float idx = begin + i * tmp;
				Vec2 loc(cos(idx), sin(idx));
				this->positions.push_back(pos + loc * factor);
				loc = loc * .5f + .5f;
				Vec4 top(loc.x * dx + colorTopLeft);
				Vec4 bot(loc.x * dy + colorBotLeft);
				this->colors.push_back(loc.y * (bot - top) + top);
			}
		}
		{
			size_t idx = base + 1;
			for (size_t i = 0; i < (points - 1); ++i)
			{
				this->indices.push_back(base);
				this->indices.push_back(idx++);
				this->indices.push_back(idx);
			}
		}
	}

	void DiskTessellator::addDisk(Vec2 pos, Vec2 size, float begin, float end, Color color, size_t points)
	{
		addDisk(pos, size, begin, end, color, color, color, color, points);
	}

	void DiskTessellator::addDisk(Vec2 pos, Vec2 size, Color colorTopLeft, Color colorTopRight, Color colorBotRight, Color colorBotLeft, size_t points)
	{
		addDisk(pos, size, 0, M_PI * 2, colorTopLeft, colorTopRight, colorBotRight, colorBotLeft, points);
	}

	void DiskTessellator::addDisk(Vec2 pos, Vec2 size, Color color, size_t points)
	{
		addDisk(pos, size, 0, M_PI * 2, color, color, color, color, points);
	}

}
