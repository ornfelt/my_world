#include <librender/Shape/RectangleBorder.h>
#include <librender/Shape/TriangleBorder.h>
#include <librender/Shape/Rectangle.h>
#include <librender/Shape/Triangle.h>
#include <librender/Shape/Circle.h>
#include <librender/Shape/Disk.h>
#include <librender/Shape/Line.h>
#include <librender/GL.h>
#include <iostream>
#include <chrono>

using librender::Color;
using librender::Vec2;

static int64_t nanotime;

#define BATCH_COUNT 3
#define TESSELLATE_COUNT 3

#define RENDER_WIDTH ((COLUMN_WIDTH + PADDING) * 7 + PADDING)
#define RENDER_HEIGHT ((PADDING + ROW_HEIGHT) * (1 + BATCH_COUNT + TESSELLATE_COUNT) + PADDING)
#define PADDING 10
#define COLUMN_WIDTH 100
#define ROW_HEIGHT 75
#define DISK_X (PADDING)
#define CIRCLE_X (DISK_X + PADDING + COLUMN_WIDTH)
#define RECTANGLE_X (CIRCLE_X + PADDING + COLUMN_WIDTH)
#define RECTANGLE_BORDER_X (RECTANGLE_X + PADDING + COLUMN_WIDTH)
#define TRIANGLE_X (RECTANGLE_BORDER_X + PADDING + COLUMN_WIDTH)
#define TRIANGLE_BORDER_X (TRIANGLE_X + PADDING + COLUMN_WIDTH)
#define LINE_X (TRIANGLE_BORDER_X + PADDING + COLUMN_WIDTH)

static void renderDisk()
{
	//Simple
	{
		librender::Disk disk;
		disk.setPoints(20);
		disk.setPos(DISK_X + COLUMN_WIDTH / 2, PADDING + ROW_HEIGHT / 2);
		disk.setSize(Vec2(COLUMN_WIDTH, ROW_HEIGHT));
		disk.setTopLeftColor(Color::RED);
		disk.setTopRightColor(Color::BLUE);
		disk.setBotRightColor(Color::GREEN);
		disk.setBotLeftColor(Color::BLACK);
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		if (there > M_PI * 2)
		{
			disk.setEnd(M_PI * 2);
			disk.setBegin(there);
		}
		else
		{
			disk.setBegin(there);
			disk.setEnd(M_PI * 2);
		}
		disk.setScale(fabs(sin(there / 2)) * .5 + .5);
		disk.draw();
	}
	//Batched
	{
		librender::DiskBatch batch;
		batch.setPos(DISK_X, PADDING + (PADDING + ROW_HEIGHT) * 1 + ROW_HEIGHT * .5);
		std::vector<librender::DiskBatched> batched(BATCH_COUNT);
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < batched.size(); ++i)
		{
			batched[i].setPoints(20);
			batched[i].setPos(COLUMN_WIDTH / 2, (PADDING + ROW_HEIGHT) * i);
			batched[i].setSize(Vec2(COLUMN_WIDTH, ROW_HEIGHT));
			batched[i].setTopLeftColor(Color::RED);
			batched[i].setTopRightColor(Color::BLUE);
			batched[i].setBotRightColor(Color::GREEN);
			batched[i].setBotLeftColor(Color::BLACK);
			if (there > M_PI * 2)
			{
				batched[i].setEnd(M_PI * 2);
				batched[i].setBegin(there);
			}
			else
			{
				batched[i].setBegin(there);
				batched[i].setEnd(M_PI * 2);
			}
			batched[i].setScale(fabs(sin(there / 2)) * .5 + .5);
			batch.addChild(&batched[i]);
		}
		batch.draw();
	}
	//Tessellated
	{
		librender::DiskTessellator tessellator;
		tessellator.setPos(DISK_X, PADDING + (PADDING + ROW_HEIGHT) * (1 + BATCH_COUNT) + ROW_HEIGHT * .5);
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < TESSELLATE_COUNT; ++i)
		{
			Vec2 pos(COLUMN_WIDTH / 2, (PADDING + ROW_HEIGHT) * i);
			Vec2 size(COLUMN_WIDTH, ROW_HEIGHT);
			size *= fabs(sin(there / 2)) * .5 + .5;
			float begin;
			float end;
			if (there > M_PI * 2)
			{
				end = M_PI * 2;
				begin = there;
			}
			else
			{
				begin = there;
				end = M_PI * 2;
			}
			tessellator.addDisk(pos, size, begin, end, Color::RED, Color::BLUE, Color::GREEN, Color::BLACK, 20);
		}
		tessellator.draw();
	}
}

static void renderCircle()
{
	glLineWidth(1);
	//Simple
	{
		librender::Circle circle;
		circle.setPoints(20);
		circle.setPos(CIRCLE_X + COLUMN_WIDTH / 2, PADDING + ROW_HEIGHT / 2);
		circle.setSize(Vec2(COLUMN_WIDTH, ROW_HEIGHT));
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		if (there > M_PI * 2)
		{
			circle.setEnd(M_PI * 2);
			circle.setBegin(there);
			circle.setBeginColor(Color::BLUE);
			circle.setEndColor(Color::RED);
		}
		else
		{
			circle.setBegin(there);
			circle.setEnd(M_PI * 2);
			circle.setBeginColor(Color::RED);
			circle.setEndColor(Color::BLUE);
		}
		circle.setScale(fabs(sin(there / 2)) * .5 + .5);
		circle.draw();
	}
	//Batched
	{
		librender::CircleBatch batch;
		batch.setPos(CIRCLE_X, PADDING + (PADDING + ROW_HEIGHT) * 1 + ROW_HEIGHT * .5);
		std::vector<librender::CircleBatched> batched(BATCH_COUNT);
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < batched.size(); ++i)
		{
			batched[i].setPoints(20);
			batched[i].setPos(COLUMN_WIDTH / 2, (PADDING + ROW_HEIGHT) * i);
			batched[i].setSize(Vec2(COLUMN_WIDTH, ROW_HEIGHT));
			if (there > M_PI * 2)
			{
				batched[i].setEnd(M_PI * 2);
				batched[i].setBegin(there);
				batched[i].setBeginColor(Color::BLUE);
				batched[i].setEndColor(Color::RED);
			}
			else
			{
				batched[i].setBegin(there);
				batched[i].setEnd(M_PI * 2);
				batched[i].setBeginColor(Color::RED);
				batched[i].setEndColor(Color::BLUE);
			}
			batched[i].setScale(fabs(sin(there / 2)) * .5 + .5);
			batch.addChild(&batched[i]);
		}
		batch.draw();
	}
	//Tessellated
	{
		librender::CircleTessellator tessellator;
		tessellator.setPos(CIRCLE_X, PADDING + (PADDING + ROW_HEIGHT) * (1 + BATCH_COUNT) + ROW_HEIGHT * .5);
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < TESSELLATE_COUNT; ++i)
		{
			Vec2 pos(COLUMN_WIDTH / 2, (PADDING + ROW_HEIGHT) * i);
			Vec2 size(COLUMN_WIDTH, ROW_HEIGHT);
			size *= fabs(sin(there / 2)) * .5 + .5;
			float begin;
			float end;
			Color beginColor;
			Color endColor;
			if (there > M_PI * 2)
			{
				end = M_PI * 2;
				begin = there;
				beginColor = Color::BLUE;
				endColor = Color::RED;
			}
			else
			{
				begin = there;
				end = M_PI * 2;
				beginColor = Color::RED;
				endColor = Color::BLUE;
			}
			tessellator.addCircle(pos, size, begin, end, beginColor, endColor, 20);
		}
		tessellator.draw();
	}
}

static void renderRectangle()
{
	//Simple
	{
		librender::Rectangle rectangle;
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		if (there > M_PI * 2)
		{
			float offset = 1 - (cos(there - M_PI * 2) * .5 + .5);
			rectangle.setPos(RECTANGLE_X, PADDING);
			rectangle.setSize(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT));
		}
		else
		{
			float offset = 1 - (cos(there) * .5 + .5);
			rectangle.setPos(RECTANGLE_X + offset * COLUMN_WIDTH, PADDING + offset * ROW_HEIGHT);
			rectangle.setSize(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT));
		}
		rectangle.setTopLeftColor(Color::RED);
		rectangle.setTopRightColor(Color::BLUE);
		rectangle.setBotRightColor(Color::GREEN);
		rectangle.setBotLeftColor(Color::BLACK);
		rectangle.setScale(fabs(sin(there / 2)) * .5 + .5);
		rectangle.draw();
	}
	//Batched
	{
		librender::RectangleBatch batch;
		batch.setPos(RECTANGLE_X, PADDING * 2 + ROW_HEIGHT * 1);
		std::vector<librender::RectangleBatched> batched(BATCH_COUNT);
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < batched.size(); ++i)
		{
			if (there > M_PI * 2)
			{
				float offset = 1 - (cos(there - M_PI * 2) * .5 + .5);
				batched[i].setPos(0, (PADDING + ROW_HEIGHT) * i);
				batched[i].setSize(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT));
			}
			else
			{
				float offset = 1 - (cos(there) * .5 + .5);
				batched[i].setPos(offset * COLUMN_WIDTH, offset * ROW_HEIGHT + (PADDING + ROW_HEIGHT) * i);
				batched[i].setSize(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT));
			}
			batched[i].setTopLeftColor(Color::RED);
			batched[i].setTopRightColor(Color::BLUE);
			batched[i].setBotRightColor(Color::GREEN);
			batched[i].setBotLeftColor(Color::BLACK);
			batched[i].setScale(fabs(sin(there / 2)) * .5 + .5);
			batch.addChild(&batched[i]);
		}
		batch.draw();
	}
	//Tessellated
	{
		librender::RectangleTessellator tessellator;
		tessellator.setPos(RECTANGLE_X, PADDING + (PADDING + ROW_HEIGHT) * (1 + BATCH_COUNT));
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < TESSELLATE_COUNT; ++i)
		{
			Vec2 pos;
			Vec2 size;
			if (there > M_PI * 2)
			{
				float offset = 1 - (cos(there - M_PI * 2) * .5 + .5);
				pos = Vec2(0, (PADDING + ROW_HEIGHT) * i);
				size = Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT);
			}
			else
			{
				float offset = 1 - (cos(there) * .5 + .5);
				pos = Vec2(offset * COLUMN_WIDTH, offset * ROW_HEIGHT + (PADDING + ROW_HEIGHT) * i);
				size = Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT);
			}
			float scale = fabs(sin(there / 2)) * .5 + .5;
			size *= scale;
			tessellator.addRectangle(pos, size, Color::RED, Color::BLUE, Color::GREEN, Color::BLACK);
		}
		tessellator.draw();
	}
}

static void renderRectangleBorder()
{
	glLineWidth(1);
	//Simple
	{
		librender::RectangleBorder rectangleBorder;
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		if (there > M_PI * 2)
		{
			float offset = 1 - (cos(there - M_PI * 2) * .5 + .5);
			rectangleBorder.setPos(RECTANGLE_BORDER_X, PADDING);
			rectangleBorder.setSize(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT));
		}
		else
		{
			float offset = 1 - (cos(there) * .5 + .5);
			rectangleBorder.setPos(RECTANGLE_BORDER_X + offset * COLUMN_WIDTH, PADDING + offset * ROW_HEIGHT);
			rectangleBorder.setSize(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT));
		}
		rectangleBorder.setTopLeftColor(Color::RED);
		rectangleBorder.setTopRightColor(Color::BLUE);
		rectangleBorder.setBotRightColor(Color::GREEN);
		rectangleBorder.setBotLeftColor(Color::BLACK);
		rectangleBorder.setScale(fabs(sin(there / 2)) * .5 + .5);
		rectangleBorder.draw();
	}
	//Batched
	{
		librender::RectangleBorderBatch batch;
		batch.setPos(RECTANGLE_BORDER_X, PADDING * 2 + ROW_HEIGHT * 1);
		std::vector<librender::RectangleBorderBatched> batched(BATCH_COUNT);
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < batched.size(); ++i)
		{
			if (there > M_PI * 2)
			{
				float offset = 1 - (cos(there - M_PI * 2) * .5 + .5);
				batched[i].setPos(0, (PADDING + ROW_HEIGHT) * i);
				batched[i].setSize(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT));
			}
			else
			{
				float offset = 1 - (cos(there) * .5 + .5);
				batched[i].setPos(offset * COLUMN_WIDTH, (PADDING + ROW_HEIGHT) * i + offset * ROW_HEIGHT);
				batched[i].setSize(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT));
			}
			batched[i].setTopLeftColor(Color::RED);
			batched[i].setTopRightColor(Color::BLUE);
			batched[i].setBotRightColor(Color::GREEN);
			batched[i].setBotLeftColor(Color::BLACK);
			batched[i].setScale(fabs(sin(there / 2)) * .5 + .5);
			batch.addChild(&batched[i]);
		}
		batch.draw();
	}
	//Tessellated
	{
		librender::RectangleBorderTessellator tessellator;
		tessellator.setPos(RECTANGLE_BORDER_X, PADDING + (PADDING + ROW_HEIGHT) * (1 + BATCH_COUNT));
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < TESSELLATE_COUNT; ++i)
		{
			Vec2 pos;
			Vec2 size;
			if (there > M_PI * 2)
			{
				float offset = 1 - (cos(there - M_PI * 2) * .5 + .5);
				pos = Vec2(0, (PADDING + ROW_HEIGHT) * i);
				size = Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT);
			}
			else
			{
				float offset = 1 - (cos(there) * .5 + .5);
				pos = Vec2(offset * COLUMN_WIDTH, offset * ROW_HEIGHT + (PADDING + ROW_HEIGHT) * i);
				size = Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, ROW_HEIGHT - offset * ROW_HEIGHT);
			}
			float scale = fabs(sin(there / 2)) * .5 + .5;
			size *= scale;
			tessellator.addRectangleBorder(pos, size, Color::RED, Color::BLUE, Color::GREEN, Color::BLACK);
		}
		tessellator.draw();
	}
}

static void renderTriangle()
{
	//Simple
	{
		librender::Triangle triangle;
		float there = fmod(nanotime / 1000000000. * M_PI, M_PI * 8);
		triangle.setPoint1(Vec2(cos(there) * COLUMN_WIDTH * .5                   , sin(there) * .5 * ROW_HEIGHT));
		triangle.setPoint2(Vec2(cos(there + M_PI * .66666666) * COLUMN_WIDTH * .5, sin(there + M_PI * .6666666) * .5 * ROW_HEIGHT));
		triangle.setPoint3(Vec2(cos(there + M_PI * 1.3333333) * COLUMN_WIDTH * .5, sin(there + M_PI * 1.333333) * .5 * ROW_HEIGHT));
		triangle.setPos(TRIANGLE_X + COLUMN_WIDTH / 2, PADDING + ROW_HEIGHT / 2);
		triangle.setColor1(Color::RED);
		triangle.setColor2(Color::GREEN);
		triangle.setColor3(Color::BLUE);
		triangle.setScale(fabs(sin(there / 2)) * .5 + .5);
		triangle.draw();
	}
	//Batched
	{
		librender::TriangleBatch batch;
		batch.setPos(TRIANGLE_X, PADDING * 2 + ROW_HEIGHT * 1);
		std::vector<librender::TriangleBatched> batched(BATCH_COUNT);
		float there = fmod(nanotime / 1000000000. * M_PI, M_PI * 8);
		for (size_t i = 0; i < batched.size(); ++i)
		{
			batched[i].setPoint1(Vec2(cos(there) * COLUMN_WIDTH * .5                   , sin(there) * .5 * ROW_HEIGHT));
			batched[i].setPoint2(Vec2(cos(there + M_PI * .66666666) * COLUMN_WIDTH * .5, sin(there + M_PI * .6666666) * .5 * ROW_HEIGHT));
			batched[i].setPoint3(Vec2(cos(there + M_PI * 1.3333333) * COLUMN_WIDTH * .5, sin(there + M_PI * 1.333333) * .5 * ROW_HEIGHT));
			batched[i].setPos(COLUMN_WIDTH / 2, (PADDING + ROW_HEIGHT) * i + ROW_HEIGHT / 2);
			batched[i].setColor1(Color::RED);
			batched[i].setColor2(Color::GREEN);
			batched[i].setColor3(Color::BLUE);
			batched[i].setScale(fabs(sin(there / 2)) * .5 + .5);
			batch.addChild(&batched[i]);
		}
		batch.draw();
	}
	//Tessellated
	{
		librender::TriangleTessellator tessellator;
		tessellator.setPos(TRIANGLE_X, PADDING + (PADDING + ROW_HEIGHT) * (1 + BATCH_COUNT));
		float there = fmod(nanotime / 1000000000. * M_PI, M_PI * 8);
		for (size_t i = 0; i < TESSELLATE_COUNT; ++i)
		{
			Vec2 p1(cos(there) * COLUMN_WIDTH * .5                   , sin(there) * .5 * ROW_HEIGHT);
			Vec2 p2(cos(there + M_PI * .66666666) * COLUMN_WIDTH * .5, sin(there + M_PI * .6666666) * .5 * ROW_HEIGHT);
			Vec2 p3(cos(there + M_PI * 1.3333333) * COLUMN_WIDTH * .5, sin(there + M_PI * 1.333333) * .5 * ROW_HEIGHT);
			Vec2 pos(COLUMN_WIDTH / 2, (PADDING + ROW_HEIGHT) * i + ROW_HEIGHT / 2);
			float scale = fabs(sin(there / 2)) * .5 + .5;
			tessellator.addTriangle(p1 * scale + pos, p2 * scale + pos, p3 * scale + pos, Color::RED, Color::GREEN, Color::BLUE);
		}
		tessellator.draw();
	}
}

static void renderTriangleBorder()
{
	//Simple
	{
		librender::TriangleBorder triangleBorder;
		float there = fmod(nanotime / 1000000000. * M_PI, M_PI * 8);
		triangleBorder.setPoint1(Vec2(cos(there) * COLUMN_WIDTH * .5                   , sin(there) * .5 * ROW_HEIGHT));
		triangleBorder.setPoint2(Vec2(cos(there + M_PI * .66666666) * COLUMN_WIDTH * .5, sin(there + M_PI * .6666666) * .5 * ROW_HEIGHT));
		triangleBorder.setPoint3(Vec2(cos(there + M_PI * 1.3333333) * COLUMN_WIDTH * .5, sin(there + M_PI * 1.333333) * .5 * ROW_HEIGHT));
		triangleBorder.setPos(TRIANGLE_BORDER_X + COLUMN_WIDTH / 2, PADDING + ROW_HEIGHT / 2);
		triangleBorder.setColor1(Color::RED);
		triangleBorder.setColor2(Color::GREEN);
		triangleBorder.setColor3(Color::BLUE);
		triangleBorder.setScale(fabs(sin(there / 2)) * .5 + .5);
		triangleBorder.draw();
	}
	//Batched
	{
		librender::TriangleBorderBatch batch;
		batch.setPos(TRIANGLE_BORDER_X, PADDING * 2 + ROW_HEIGHT * 1);
		std::vector<librender::TriangleBorderBatched> batched(BATCH_COUNT);
		float there = fmod(nanotime / 1000000000. * M_PI, M_PI * 8);
		for (size_t i = 0; i < batched.size(); ++i)
		{
			batched[i].setPoint1(Vec2(cos(there) * COLUMN_WIDTH * .5                   , sin(there) * .5 * ROW_HEIGHT));
			batched[i].setPoint2(Vec2(cos(there + M_PI * .66666666) * COLUMN_WIDTH * .5, sin(there + M_PI * .6666666) * .5 * ROW_HEIGHT));
			batched[i].setPoint3(Vec2(cos(there + M_PI * 1.3333333) * COLUMN_WIDTH * .5, sin(there + M_PI * 1.333333) * .5 * ROW_HEIGHT));
			batched[i].setPos(COLUMN_WIDTH / 2, (PADDING + ROW_HEIGHT) * i + ROW_HEIGHT / 2);
			batched[i].setColor1(Color::RED);
			batched[i].setColor2(Color::GREEN);
			batched[i].setColor3(Color::BLUE);
			batched[i].setScale(fabs(sin(there / 2)) * .5 + .5);
			batch.addChild(&batched[i]);
		}
		batch.draw();
	}
	//Tessellated
	{
		librender::TriangleBorderTessellator tessellator;
		tessellator.setPos(TRIANGLE_BORDER_X, PADDING + (PADDING + ROW_HEIGHT) * (1 + BATCH_COUNT));
		float there = fmod(nanotime / 1000000000. * M_PI, M_PI * 8);
		for (size_t i = 0; i < TESSELLATE_COUNT; ++i)
		{
			Vec2 p1(cos(there) * COLUMN_WIDTH * .5                   , sin(there) * .5 * ROW_HEIGHT);
			Vec2 p2(cos(there + M_PI * .66666666) * COLUMN_WIDTH * .5, sin(there + M_PI * .6666666) * .5 * ROW_HEIGHT);
			Vec2 p3(cos(there + M_PI * 1.3333333) * COLUMN_WIDTH * .5, sin(there + M_PI * 1.333333) * .5 * ROW_HEIGHT);
			Vec2 pos(COLUMN_WIDTH / 2, (PADDING + ROW_HEIGHT) * i + ROW_HEIGHT / 2);
			float scale = fabs(sin(there / 2)) * .5 + .5;
			tessellator.addTriangleBorder(p1 * scale + pos, p2 * scale + pos, p3 * scale + pos, Color::RED, Color::GREEN, Color::BLUE);
		}
		tessellator.draw();
	}
}

static void renderLine()
{
	glLineWidth(10);
	//Simple
	{
		librender::Line line;
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		if (there > M_PI * 2)
		{
			float offset = 1 - (cos(there - M_PI * 2) * .5 + .5);
			line.setPoint1(Vec2(0, 0));
			line.setPoint2(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, 0));
		}
		else
		{
			float offset = 1 - (cos(there) * .5 + .5);
			line.setPoint1(Vec2(offset * COLUMN_WIDTH, 0));
			line.setPoint2(Vec2(COLUMN_WIDTH, 0));
		}
		line.setPos(LINE_X, PADDING + ROW_HEIGHT / 2);
		line.setColor1(Color::BLUE);
		line.setColor2(Color::RED);
		line.setScale(fabs(sin(there / 2)) * .5 + .5);
		line.draw();
	}
	//Batched
	{
		librender::LineBatch batch;
		batch.setPos(LINE_X, PADDING * 2 + ROW_HEIGHT * 1);
		std::vector<librender::LineBatched> batched(BATCH_COUNT);
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < batched.size(); ++i)
		{
			if (there > M_PI * 2)
			{
				float offset = 1 - (cos(there - M_PI * 2) * .5 + .5);
				batched[i].setPoint1(Vec2(0, 0));
				batched[i].setPoint2(Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, 0));
			}
			else
			{
				float offset = 1 - (cos(there) * .5 + .5);
				batched[i].setPoint1(Vec2(offset * COLUMN_WIDTH, 0));
				batched[i].setPoint2(Vec2(COLUMN_WIDTH, 0));
			}
			batched[i].setPos(0, (PADDING + ROW_HEIGHT) * i + ROW_HEIGHT / 2);
			batched[i].setColor1(Color::BLUE);
			batched[i].setColor2(Color::RED);
			batched[i].setScale(fabs(sin(there / 2)) * .5 + .5);
			batch.addChild(&batched[i]);
		}
		batch.draw();
	}
	//Tessellated
	{
		librender::LineTessellator tessellator;
		tessellator.setPos(LINE_X, PADDING + (PADDING + ROW_HEIGHT) * (1 + BATCH_COUNT));
		float there = fmod(nanotime / 1000000000. * M_PI * 2, M_PI * 4);
		for (size_t i = 0; i < TESSELLATE_COUNT; ++i)
		{
			Vec2 p1;
			Vec2 p2;
			if (there > M_PI * 2)
			{
				float offset = 1 - (cos(there - M_PI * 2) * .5 + .5);
				p1 = Vec2(0, 0);
				p2 = Vec2(COLUMN_WIDTH - offset * COLUMN_WIDTH, 0);
			}
			else
			{
				float offset = 1 - (cos(there) * .5 + .5);
				p1 = Vec2(offset * COLUMN_WIDTH, 0);
				p2 = Vec2(COLUMN_WIDTH, 0);
			}
			Vec2 pos(0, (PADDING + ROW_HEIGHT) * i + ROW_HEIGHT / 2);
			float scale = fabs(sin(there / 2)) * .5 + .5;
			tessellator.addLine(p1 * scale + pos, p2 * scale + pos, Color::BLUE, Color::RED);
		}
		tessellator.draw();
	}
}

static void render()
{
	renderDisk();
	renderCircle();
	renderRectangle();
	renderRectangleBorder();
	renderTriangle();
	renderTriangleBorder();
	renderLine();
}

int main()
{
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(RENDER_WIDTH, RENDER_HEIGHT, "osef", NULL, NULL);
	if (!window)
		return 0;
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "failed to initialize OpenGL context" << std::endl;
		return EXIT_FAILURE;
	}
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glViewport(0, 0, RENDER_WIDTH, RENDER_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, RENDER_WIDTH, RENDER_HEIGHT, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(.1, .1, .1, 1);
	while (!glfwWindowShouldClose(window))
	{
		nanotime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch()).count();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();
		render();
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return EXIT_SUCCESS;
}
