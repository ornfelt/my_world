#ifndef LIBRENDER_CIRCLE_H
# define LIBRENDER_CIRCLE_H

# include "../Color.h"
# include "./Shape.h"

namespace librender
{

	class CircleBase : public ShapeBase
	{

	private:
		Vec4 colorPoints[2];
		Vec2 size;
		float begin;
		float end;
		void updatePositions();
		void updateIndices();
		void updateColors();

	public:
		CircleBase();
		void setPoints(size_t count);
		void setColor(Color color);
		void setBeginColor(Color color);
		void setEndColor(Color color);
		void setWidth(float width);
		float getWidth();
		void setHeight(float height);
		float getHeight();
		void setSize(Vec2 size);
		void setBegin(float begin);
		void setEnd(float end);

	};

	class Circle : public CircleBase, public Shape
	{

	public:
		Circle();

	};

	class CircleBatched : public CircleBase, public ShapeBatched
	{
	};

	class CircleBatch : public ShapeBatch
	{

	public:
		CircleBatch();

	};

	class CircleTessellator : public ShapeTessellator
	{

	public:
		CircleTessellator();
		void addCircle(Vec2 pos, Vec2 size, float begin, float end, Color beginColor, Color endColor, size_t points = 0);
		void addCircle(Vec2 pos, Vec2 size, float begin, float end, Color color, size_t points = 0);
		void addCircle(Vec2 pos, Vec2 size, Color beginColor, Color endColor, size_t points = 0);
		void addCircle(Vec2 pos, Vec2 size, Color color, size_t points = 0);

	};

}

#endif
