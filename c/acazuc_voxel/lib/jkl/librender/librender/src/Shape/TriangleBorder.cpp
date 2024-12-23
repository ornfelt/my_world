#include "TriangleBorder.h"
#include "../DrawableBuffers.h"
#include "../GL.h"

namespace librender
{

	TriangleBorderBase::TriangleBorderBase()
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
		this->verticesNumber = 3;
		this->colors.resize(3, Vec4(1));
		this->positions.resize(3, Vec2(0));
		this->indicesNumber = 6;
		this->indices.resize(6);
		this->indices[0] = 0;
		this->indices[1] = 1;
		this->indices[2] = 1;
		this->indices[3] = 2;
		this->indices[4] = 2;
		this->indices[5] = 0;
	}

	void TriangleBorderBase::setColor1(Color color)
	{
		this->colors[0] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBorderBase::setColor2(Color color)
	{
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBorderBase::setColor3(Color color)
	{
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBorderBase::setColors(Color color1, Color color2, Color color3)
	{
		this->colors[0] = color1;
		this->colors[1] = color2;
		this->colors[2] = color3;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBorderBase::setColor(Color color)
	{
		this->colors[0] = color;
		this->colors[1] = color;
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBorderBase::setPoint1(Vec2 pos)
	{
		this->positions[0] = pos;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void TriangleBorderBase::setPoint2(Vec2 pos)
	{
		this->positions[1] = pos;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void TriangleBorderBase::setPoint3(Vec2 pos)
	{
		this->positions[2] = pos;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void TriangleBorderBase::setPoints(Vec2 pos1, Vec2 pos2, Vec2 pos3)
	{
		this->positions[0] = pos1;
		this->positions[1] = pos2;
		this->positions[2] = pos3;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	TriangleBorder::TriangleBorder()
	: Shape(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	TriangleBorderBatch::TriangleBorderBatch()
	: ShapeBatch(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	TriangleBorderTessellator::TriangleBorderTessellator()
	: ShapeTessellator(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_INDICES | DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	void TriangleBorderTessellator::addTriangleBorder(Vec2 pos1, Vec2 pos2, Vec2 pos3, Color color1, Color color2, Color color3)
	{
		size_t base = this->positions.size();
		this->positions.push_back(pos1);
		this->positions.push_back(pos2);
		this->positions.push_back(pos3);
		this->indices.push_back(base + 0);
		this->indices.push_back(base + 1);
		this->indices.push_back(base + 1);
		this->indices.push_back(base + 2);
		this->indices.push_back(base + 2);
		this->indices.push_back(base + 0);
		this->colors.push_back(color1);
		this->colors.push_back(color2);
		this->colors.push_back(color3);
	}

	void TriangleBorderTessellator::addTriangleBorder(Vec2 pos1, Vec2 pos2, Vec2 pos3, Color color)
	{
		addTriangleBorder(pos1, pos2, pos3, color, color, color);
	}

}
