#ifndef LIBRENDER_TRIANGLE_H
# define LIBRENDER_TRIANGLE_H

# include "../Color.h"
# include "./Shape.h"

namespace librender
{

	class TriangleBase : public ShapeBase
	{

	public:
		TriangleBase();
		void setColor1(Color color);
		void setColor2(Color color);
		void setColor3(Color color);
		void setColor(Color color);
		void setColors(Color color1, Color color2, Color color3);
		void setPoint1(Vec2 pos);
		void setPoint2(Vec2 pos);
		void setPoint3(Vec2 pos);
		void setPoints(Vec2 pos1, Vec2 pos2, Vec2 pos3);

	};

	class Triangle : public TriangleBase, public Shape
	{

	public:
		Triangle();

	};

	class TriangleBatched : public TriangleBase, public ShapeBatched
	{
	};

	class TriangleBatch : public ShapeBatch
	{

	public:
		TriangleBatch();

	};

	class TriangleTessellator : public ShapeTessellator
	{

	public:
		TriangleTessellator();
		void addTriangle(Vec2 pos1, Vec2 pos2, Vec2 pos3, Color color1, Color color2, Color color3);
		void addTriangle(Vec2 pos1, Vec2 pos2, Vec2 pos3, Color color);

	};

}

#endif
