#ifndef LIBRENDER_LINE_H
# define LIBRENDER_LINE_H

# include "../Color.h"
# include "./Shape.h"

namespace librender
{

	class LineBase : public ShapeBase
	{

	public:
		LineBase();
		void setColor1(Color color);
		void setColor2(Color color);
		void setColors(Color color1, Color color2);
		void setColor(Color color);
		void setPoint1(Vec2 pos);
		void setPoint2(Vec2 pos);
		void setPoints(Vec2 pos1, Vec2 pos2);

	};

	class Line : public LineBase, public Shape
	{

	public:
		Line();

	};

	class LineBatched : public LineBase, public ShapeBatched
	{
	};

	class LineBatch : public ShapeBatch
	{

	public:
		LineBatch();

	};

	class LineTessellator : public ShapeTessellator
	{

	public:
		LineTessellator();
		void addLine(Vec2 pos1, Vec2 pos2, Color color1, Color color2);
		void addLine(Vec2 pos1, Vec2 pos2, Color color);

	};

	class LineStripTessellator : public ShapeTessellator
	{

	public:
		LineStripTessellator();
		void addPoint(Vec2 pos1, Color color);

	};

}

#endif
