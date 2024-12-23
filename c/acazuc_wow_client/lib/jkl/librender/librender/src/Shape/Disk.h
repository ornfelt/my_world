#ifndef LIBRENDER_DISK_H
# define LIBRENDER_DISK_H

# include "../Color.h"
# include "./Shape.h"

namespace librender
{

	class DiskBase : public ShapeBase
	{

	private:
		Vec4 colorPoints[4];
		Vec2 size;
		float begin;
		float end;
		void updatePositions();
		void updateIndices();
		void updateColors();

	public:
		DiskBase();
		void setPoints(size_t count);
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
		void setBegin(float begin);
		void setEnd(float end);

	};

	class Disk : public DiskBase, public Shape
	{

	public:
		Disk();

	};

	class DiskBatched : public DiskBase, public ShapeBatched
	{
	};

	class DiskBatch : public ShapeBatch
	{

	public:
		DiskBatch();

	};

	class DiskTessellator : public ShapeTessellator
	{

	public:
		DiskTessellator();
		void addDisk(Vec2 pos, Vec2 size, float begin, float end, Color colorTopLeft, Color colorTopRight, Color colorBotRight, Color colorBotLeft, size_t points = 0);
		void addDisk(Vec2 pos, Vec2 size, float begin, float end, Color color, size_t points = 0);
		void addDisk(Vec2 pos, Vec2 size, Color colorTopLeft, Color colorTopRight, Color colorBotRight, Color colorBotLeft, size_t points = 0);
		void addDisk(Vec2 pos, Vec2 size, Color color, size_t points = 0);

	};

}

#endif
