#ifndef LIBRENDER_RECTANGLE_BORDER_H
# define LIBRENDER_RECTANGLE_BORDER_H

# include "../Color.h"
# include "./Shape.h"

namespace librender
{

	class RectangleBorderBase : public ShapeBase
	{

	private:
		Vec2 size;
		void updatePositions();

	public:
		RectangleBorderBase();
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

	class RectangleBorder : public RectangleBorderBase, public Shape
	{

	public:
		RectangleBorder();

	};

	class RectangleBorderBatched : public RectangleBorderBase, public ShapeBatched
	{
	};

	class RectangleBorderBatch : public ShapeBatch
	{

	public:
		RectangleBorderBatch();

	};

	class RectangleBorderTessellator : public ShapeTessellator
	{

	public:
		RectangleBorderTessellator();
		void addRectangleBorder(Vec2 pos, Vec2 size, Color colorTopLeft, Color colorTopRight, Color colorBotLeft, Color colorBotRight);
		void addRectangleBorder(Vec2 pos, Vec2 size, Color color);

	};

}

#endif
