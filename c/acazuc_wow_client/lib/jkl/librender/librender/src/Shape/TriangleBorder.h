#ifndef LIBRENDER_TRIANGLE_BORDER_H
# define LIBRENDER_TRIANGLE_BORDER_H

# include "../Color.h"
# include "./Shape.h"

namespace librender
{

	class TriangleBorderBase : public ShapeBase
	{

	public:
		TriangleBorderBase();
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

	class TriangleBorder : public TriangleBorderBase, public Shape
	{

	public:
		TriangleBorder();

	};

	class TriangleBorderBatched : public TriangleBorderBase, public ShapeBatched
	{
	};

	class TriangleBorderBatch : public ShapeBatch
	{

	public:
		TriangleBorderBatch();

	};

	class TriangleBorderTessellator : public ShapeTessellator
	{

	public:
		TriangleBorderTessellator();
		void addTriangleBorder(Vec2 pos1, Vec2 pos2, Vec2 pos3, Color color1, Color color2, Color color3);
		void addTriangleBorder(Vec2 pos1, Vec2 pos2, Vec2 pos3, Color color);

	};

}

#endif
