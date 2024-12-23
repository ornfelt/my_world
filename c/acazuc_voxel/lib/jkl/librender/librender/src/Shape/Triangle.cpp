#include "Triangle.h"
#include "../DrawableBuffers.h"

namespace librender
{

	TriangleBase::TriangleBase()
	{
		this->buffers = DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
		this->verticesNumber = 3;
		this->colors.resize(3, Vec4(1));
		this->positions.resize(3, Vec2(0));
	}

	void TriangleBase::setColor1(Color color)
	{
		this->colors[0] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBase::setColor2(Color color)
	{
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBase::setColor3(Color color)
	{
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBase::setColors(Color color1, Color color2, Color color3)
	{
		this->colors[0] = color1;
		this->colors[1] = color2;
		this->colors[2] = color3;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBase::setColor(Color color)
	{
		this->colors[0] = color;
		this->colors[1] = color;
		this->colors[2] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void TriangleBase::setPoint1(Vec2 pos)
	{
		this->positions[0] = pos;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void TriangleBase::setPoint2(Vec2 pos)
	{
		this->positions[1] = pos;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void TriangleBase::setPoint3(Vec2 pos)
	{
		this->positions[2] = pos;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void TriangleBase::setPoints(Vec2 pos1, Vec2 pos2, Vec2 pos3)
	{
		this->positions[0] = pos1;
		this->positions[1] = pos2;
		this->positions[2] = pos3;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	Triangle::Triangle()
	{
		this->buffers = DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	TriangleBatch::TriangleBatch()
	{
		this->buffers = DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	TriangleTessellator::TriangleTessellator()
	{
		this->buffers = DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	void TriangleTessellator::addTriangle(Vec2 pos1, Vec2 pos2, Vec2 pos3, Color color1, Color color2, Color color3)
	{
		this->positions.push_back(pos1);
		this->positions.push_back(pos2);
		this->positions.push_back(pos3);
		this->colors.push_back(color1);
		this->colors.push_back(color2);
		this->colors.push_back(color3);
	}

	void TriangleTessellator::addTriangle(Vec2 pos1, Vec2 pos2, Vec2 pos3, Color color)
	{
		addTriangle(pos1, pos2, pos3, color, color, color);
	}

}
