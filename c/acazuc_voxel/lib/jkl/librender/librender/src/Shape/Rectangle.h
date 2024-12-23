#ifndef LIBRENDER_RECTANGLE_H
# define LIBRENDER_RECTANGLE_H

# include "../Color.h"
# include "./Shape.h"

namespace librender
{

	class RectangleBase : public ShapeBase
	{

	private:
		Vec2 size;
		void updatePositions();

	public:
		RectangleBase();
		void setColor(Color color);
		void setTopColor(Color color);
		void setBotColor(Color color);
		void setLeftColor(Color color);
		void setRightColor(Color color);
		void setTopLeftColor(Color color);
		void setTopRightColor(Color color);
		void setBotLeftColor(Color color);
		void setBotRightColor(Color color);
		void setWidth(float width);
		float getWidth();
		void setHeight(float height);
		float getHeight();
		void setSize(Vec2 size);
		inline void setSize(float width, float height) {setSize(Vec2(width, height));};

	};

	class Rectangle : public RectangleBase, public Shape
	{

	public:
		Rectangle();

	};

	class RectangleBatched : public RectangleBase, public ShapeBatched
	{
	};

	class RectangleBatch : public ShapeBatch
	{

	public:
		RectangleBatch();

	};

	class RectangleTessellator : public ShapeTessellator
	{

	public:
		RectangleTessellator();
		void addRectangle(Vec2 pos, Vec2 size, Color colorTopLeft, Color colorTopRight, Color colorBotLeft, Color colorBotRight);
		void addRectangle(Vec2 pos, Vec2 size, Color color);

	};

}

#endif
