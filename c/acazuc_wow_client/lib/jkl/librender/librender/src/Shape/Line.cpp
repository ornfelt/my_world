#include "Line.h"
#include "../DrawableBuffers.h"
#include "../GL.h"

namespace librender
{

	LineBase::LineBase()
	{
		this->buffers = DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
		this->verticesNumber = 2;
		this->colors.resize(2, Vec4(1));
		this->positions.resize(2, Vec2(0));
	}

	void LineBase::setColor1(Color color)
	{
		this->colors[0] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void LineBase::setColor2(Color color)
	{
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void LineBase::setColors(Color color1, Color color2)
	{
		this->colors[0] = color1;
		this->colors[1] = color2;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void LineBase::setColor(Color color)
	{
		this->colors[0] = color;
		this->colors[1] = color;
		requireUpdates(DRAWABLE_BUFFER_COLORS);
	}

	void LineBase::setPoint1(Vec2 pos)
	{
		this->positions[0] = pos;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void LineBase::setPoint2(Vec2 pos)
	{
		this->positions[1] = pos;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	void LineBase::setPoints(Vec2 pos1, Vec2 pos2)
	{
		this->positions[0] = pos1;
		this->positions[1] = pos2;
		requireUpdates(DRAWABLE_BUFFER_POSITIONS);
	}

	Line::Line()
	: Shape(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	LineBatch::LineBatch()
	: ShapeBatch(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	LineTessellator::LineTessellator()
	: ShapeTessellator(GL_LINES)
	{
		this->buffers = DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	void LineTessellator::addLine(Vec2 pos1, Vec2 pos2, Color color1, Color color2)
	{
		this->positions.push_back(pos1);
		this->positions.push_back(pos2);
		this->colors.push_back(color1);
		this->colors.push_back(color2);
	}

	void LineTessellator::addLine(Vec2 pos1, Vec2 pos2, Color color)
	{
		addLine(pos1, pos2, color, color);
	}

	LineStripTessellator::LineStripTessellator()
	: ShapeTessellator(GL_LINE_STRIP)
	{
		this->buffers = DRAWABLE_BUFFER_POSITIONS | DRAWABLE_BUFFER_COLORS;
	}

	void LineStripTessellator::addPoint(Vec2 pos, Color color)
	{
		this->positions.push_back(pos);
		this->colors.push_back(color);
	}

}
