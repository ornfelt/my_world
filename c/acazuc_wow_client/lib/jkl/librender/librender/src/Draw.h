#ifndef LIBRENDER_DRAW_H
# define LIBRENDER_DRAW_H

# include "Texture/Texture.h"
# include "Color.h"

namespace librender
{

	double getTime();
	void scissorBegin(int32_t x, int32_t y, int32_t width, int32_t height);
	void scissorEnd();
	void pushMatrix();
	void popMatrix();
	void rotate(float angle, float x, float y, float z);
	void translate(float x, float y, float z);
	void scale(float x, float y, float z);
	void drawColorQuadBegin();
	void drawColorQuadEnd();
	void drawColorQuadPart(int32_t x, int32_t y, int32_t width, int32_t height, Color topLeft, Color topRight, Color bottomRight, Color bottomLeft);
	void drawColorQuadPart(int32_t x, int32_t y, int32_t width, int32_t height, Color color);
	void drawColorQuad(int32_t x, int32_t y, int32_t width, int32_t height, Color topLeft, Color topRight, Color bottomRight, Color bottomLeft);
	void drawColorQuad(int32_t x, int32_t y, int32_t width, int32_t height, Color color);
	void drawColorQuadBorderBegin(float lineWidth = 1);
	void drawColorQuadBorderEnd();
	void drawColorQuadBorderPart(int32_t x, int32_t y, int32_t width, int32_t height, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor);
	void drawColorQuadBorderPart(int32_t x, int32_t y, int32_t width, int32_t height, Color color);
	void drawColorQuadBorder(int32_t x, int32_t y, int32_t width, int32_t height, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor, float lineWeight = 1);
	void drawColorQuadBorder(int32_t x, int32_t y, int32_t width, int32_t height, Color color, int32_t lineWeight = 1);
	void drawQuadBegin(Texture *texture);
	void drawQuadEnd();
	void drawQuadPart(Texture *texture, int32_t x, float y, int32_t width, int32_t height, int32_t texXOrg, int32_t texYOrg, int32_t texCoWidth, int32_t texCoHeight);
	void drawQuadPart(Texture *texture, int32_t x, float y, int32_t width, int32_t height);
	void drawQuadPart(Texture *texture, int32_t x, float y);
	void drawQuad(Texture *texture, int32_t x, int32_t y, int32_t width, int32_t height, int32_t texXOrg, int32_t texYOrg, int32_t texCoWidth, int32_t texCoHeight, Color color);
	void drawQuad(Texture *texture, int32_t x, int32_t y, int32_t width, int32_t height, int32_t texXOrg, int32_t texYOrg, int32_t texCoWidth, int32_t texCoHeight, float alpha = 1);
	void drawQuad(Texture *texture, int32_t x, int32_t y, int32_t width, int32_t height, Color color);
	void drawQuad(Texture *texture, int32_t x, int32_t y, int32_t width, int32_t height, float alpha = 1);
	void drawQuad(Texture *texture, int32_t x, int32_t y, Color color);
	void drawQuad(Texture *texture, int32_t x, int32_t y, float alpha = 1);
	void drawTriangleBegin();
	void drawTriangleEnd();
	void drawTrianglePart(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Color c1, Color c2, Color c3);
	void drawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Color c1, Color c2, Color c3);
	void drawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Color color);
	void drawCircleBegin(float lineWidth = 1);
	void drawCircleEnd();
	void drawCirclePart(int32_t x, int32_t y, int32_t rayon, int32_t nbSeg, float angle, float startAngle);
	void drawCirclePart(int32_t x, int32_t y, int32_t rayon, int32_t nbSeg);
	void drawCirclePart(int32_t x, int32_t y, int32_t rayon);
	void drawCircle(int32_t x, int32_t y, int32_t rayon, Color color, int32_t nbSeg, float lineWidth, float angle, float startAngle);
	void drawCircle(int32_t x, int32_t y, int32_t rayon, Color color, int32_t nbSeg, float lineWidth);
	void drawCircle(int32_t x, int32_t y, int32_t rayon, Color color, int32_t nbSeg);
	void drawCircle(int32_t x, float y, int32_t rayon, Color color);
	void drawLineBegin(float lineWidth);
	void drawLineEnd();
	void drawLinePart(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color1, Color color2);
	void drawLinePart(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color1);
	void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color1, Color color2, float lineWidth);
	void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color1, Color color2);
	void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color, float lineWidth);
	void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color);
	void drawPointBegin(float size = 1);
	void drawPointEnd();
	void drawPointPart(int32_t x, int32_t y, Color color);
	void drawPoint(int32_t x, int32_t y, Color color, float size);
	void drawPoint(float x, float y, Color color);

}

#endif
