#include "Draw.h"
#include "GL.h"
#include <cmath>

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

namespace librender
{

	double getTime()
	{
		return glfwGetTime();
	}

	void scissorBegin(int32_t x, int32_t y, int32_t width, int32_t height)
	{
		glScissor(x, y, width, height);
		glEnable(GL_SCISSOR_TEST);
	}

	void scissorEnd()
	{
		glDisable(GL_SCISSOR_TEST);
	}

	void pushMatrix()
	{
		glPushMatrix();
	}

	void popMatrix()
	{
		glPopMatrix();
	}

	void rotate(float angle, float x, float y, float z)
	{
		glRotatef(angle, x, y, z);
	}

	void translate(float x, float y, float z)
	{
		glTranslatef(x, y, z);
	}

	void scale(float x, float y, float z)
	{
		glScalef(x, y, z);
	}

	void drawColorQuadBegin()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_QUADS);
	}

	void drawColorQuadEnd()
	{
		glEnd();
	}

	void drawColorQuadPart(int32_t x, int32_t y, int32_t width, int32_t height, Color topLeft, Color topRight, Color bottomRight, Color bottomLeft)
	{
		topLeft.bind();
		glVertex2f(x, y);
		topRight.bind();
		glVertex2f(x + width, y);
		bottomRight.bind();
		glVertex2f(x + width, y + height);
		bottomLeft.bind();
		glVertex2f(x, y + height);
	}

	void drawColorQuadPart(int32_t x, int32_t y, int32_t width, int32_t height, Color color)
	{
		drawColorQuadPart(x, y, width, height, color, color, color, color);
	}

	void drawColorQuad(int32_t x, int32_t y, int32_t width, int32_t height, Color topLeft, Color topRight, Color bottomRight, Color bottomLeft)
	{
		drawColorQuadBegin();
		drawColorQuadPart(x, y, width, height, topLeft, topRight, bottomRight, bottomLeft);
		drawColorQuadEnd();
	}

	void drawColorQuad(int32_t x, int32_t y, int32_t width, int32_t height, Color color)
	{
		drawColorQuad(x, y, width, height, color, color, color, color);
	}

	void drawColorQuadBorderBegin(float lineWidth)
	{
		glDisable(GL_LINE_SMOOTH);
		glBindTexture(GL_TEXTURE_2D, 0);
		glLineWidth(lineWidth);
		glBegin(GL_LINES);
	}

	void drawColorQuadBorderEnd()
	{
		glEnd();
	}

	void drawColorQuadBorderPart(int32_t x, int32_t y, int32_t width, int32_t height, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor)
	{
		topLeftColor.bind();
		glVertex2f(x + .5f, y + .5f);
		topRightColor.bind();
		glVertex2f(x + width + .5f, y + .5f);

		topRightColor.bind();
		glVertex2f(x + width + .5f, y + .5f);
		bottomRightColor.bind();
		glVertex2f(x + width + .5f, y + height + .5f);

		bottomRightColor.bind();
		glVertex2f(x + width + .5f, y + height + .5f);
		bottomLeftColor.bind();
		glVertex2f(x + .5f, y + height + .5f);

		bottomLeftColor.bind();
		glVertex2f(x + .5f, y + height + .5f);
		topLeftColor.bind();
		glVertex2f(x + .5f, y + .5f);
	}

	void drawColorQuadBorderPart(int32_t x, int32_t y, int32_t width, int32_t height, Color color)
	{
		drawColorQuadBorderPart(x, y, width, height, color, color, color, color);
	}

	void drawColorQuadBorder(int32_t x, int32_t y, int32_t width, int32_t height, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor, float lineWidth)
	{
		drawColorQuadBorderBegin(lineWidth);
		drawColorQuadBorderPart(x, y, width, height, topLeftColor, topRightColor, bottomRightColor, bottomLeftColor);
		drawColorQuadBorderEnd();
	}

	void drawColorQuadBorder(int32_t x, int32_t y, int32_t width, int32_t height, Color color, int32_t lineWeight)
	{
		drawColorQuadBorder(x, y, width, height, color, color, color, color, lineWeight);
	}

	void drawQuadBegin(Texture *texture)
	{
		if (texture)
			texture->bind();
		glBegin(GL_QUADS);
	}

	void drawQuadEnd()
	{
		glEnd();
	}

	void drawQuadPart(Texture *texture, int32_t x, float y, int32_t width, int32_t height, int32_t texXOrg, int32_t texYOrg, int32_t texCoWidth, int32_t texCoHeight)
	{
		if (texture)
		{
			float xFrom = texXOrg / static_cast<float>(texture->getWidth());
			float xTo = (texXOrg + texCoWidth) / static_cast<float>(texture->getWidth());
			float yFrom = texYOrg / static_cast<float>(texture->getHeight());
			float yTo = (texYOrg+texCoHeight) / static_cast<float>(texture->getHeight());
			glTexCoord2f(xFrom, yFrom);
			glVertex2f(x, y);
			glTexCoord2f(xFrom, yTo);
			glVertex2f(x, y + height);
			glTexCoord2f(xTo, yTo);
			glVertex2f(x + width, y + height);
			glTexCoord2f(xTo, yFrom);
			glVertex2f(x + width, y);
		}
	}

	void drawQuadPart(Texture *texture, int32_t x, float y, int32_t width, int32_t height)
	{
		if (!texture)
			return;
		drawQuadPart(texture, x, y, width, height, 0, 0, texture->getWidth(), texture->getHeight());
	}

	void drawQuadPart(Texture *texture, int32_t x, float y)
	{
		if (!texture)
			return;
		drawQuadPart(texture, x, y, texture->getWidth(), texture->getHeight(), 0, 0, texture->getWidth(), texture->getHeight());
	}

	void drawQuad(Texture *texture, int32_t x, int32_t y, int32_t width, int32_t height, int32_t texXOrg, int32_t texYOrg, int32_t texCoWidth, int32_t texCoHeight, Color color)
	{
		drawQuadBegin(texture);
		color.bind();
		drawQuadPart(texture, x, y, width, height, texXOrg, texYOrg, texCoWidth, texCoHeight);
		drawQuadEnd();
	}

	void drawQuad(Texture *texture, int32_t x, int32_t y, int32_t width, int32_t height, int32_t texXOrg, int32_t texYOrg, int32_t texCoWidth, int32_t texCoHeight, float alpha)
	{
		drawQuadBegin(texture);
		glColor4f(1, 1, 1, alpha);
		drawQuadPart(texture, x, y, width, height, texXOrg, texYOrg, texCoWidth, texCoHeight);
		drawQuadEnd();
	}

	void drawQuad(Texture *texture, int32_t x, int32_t y, int32_t width, int32_t height, Color color)
	{
		if (!texture)
			return;
		drawQuad(texture, x, y, width, height, 0, 0, texture->getWidth(), texture->getHeight(), color);
	}

	void drawQuad(Texture *texture, int32_t x, int32_t y, int32_t width, int32_t height, float alpha)
	{
		if (!texture)
			return;
		drawQuad(texture, x, y, width, height, 0, 0, texture->getWidth(), texture->getHeight(), alpha);
	}

	void drawQuad(Texture *texture, int32_t x, int32_t y, Color color)
	{
		if (!texture)
			return;
		drawQuad(texture, x, y, texture->getWidth(), texture->getHeight(), 0, 0, texture->getWidth(), texture->getHeight(), color);
	}

	void drawQuad(Texture *texture, int32_t x, int32_t y, float alpha)
	{
		if (!texture)
			return;
		drawQuad(texture, x, y, texture->getWidth(), texture->getHeight(), 0, 0, texture->getWidth(), texture->getHeight(), alpha);
	}

	void drawTriangleBegin()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_TRIANGLES);
	}

	void drawTriangleEnd()
	{
		glEnd();
	}

	void drawTrianglePart(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Color c1, Color c2, Color c3)
	{
		c1.bind();
		glVertex2f(x1, y1);
		c2.bind();
		glVertex2f(x2, y2);
		c3.bind();
		glVertex2f(x3, y3);
	}

	void drawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Color c1, Color c2, Color c3)
	{
		drawTriangleBegin();
		drawTrianglePart(x1, y1, x2, y2, x3, y3, c1, c2, c3);
		drawTriangleEnd();
	}

	void drawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Color color)
	{
		drawTriangle(x1, y1, x2, y2, x3, y3, color, color, color);
	}

	void drawCircleBegin(float lineWidth)
	{
		glLineWidth(lineWidth);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnable(GL_LINE_SMOOTH);
		glBegin(GL_LINE_STRIP);
	}

	void drawCircleEnd()
	{
		glEnd();
		glDisable(GL_LINE_SMOOTH);
	}

	void drawCirclePart(int32_t x, int32_t y, int32_t rayon, int32_t nbSeg, float angle, float startAngle)
	{
		double step = angle / nbSeg;
		for (int32_t i = 0; i < nbSeg + 1; ++i)
			glVertex2f(rayon * cos(step * i + startAngle) + x + .5, rayon * sin(step * i + startAngle) + y + .5);
	}

	void drawCirclePart(int32_t x, int32_t y, int32_t rayon, int32_t nbSeg)
	{
		drawCirclePart(x, y, rayon, nbSeg, 2 * (float)M_PI, 0);
	}

	void drawCirclePart(int32_t x, int32_t y, int32_t rayon)
	{
		drawCirclePart(x, y, rayon, (int)(M_PI * rayon), 2 * (float)M_PI, 0);
	}

	void drawCircle(int32_t x, int32_t y, int32_t rayon, Color color, int32_t nbSeg, float lineWidth, float angle, float startAngle)
	{
		drawCircleBegin(lineWidth);
		color.bind();
		drawCirclePart(x, y, rayon, nbSeg, angle, startAngle);
		drawCircleEnd();
	}

	void drawCircle(int32_t x, int32_t y, int32_t rayon, Color color, int32_t nbSeg, float lineWidth)
	{
		drawCircle(x, y, rayon, color, nbSeg, lineWidth, 2 * (float)M_PI, 0);
	}

	void drawCircle(int32_t x, int32_t y, int32_t rayon, Color color, int32_t nbSeg)
	{
		drawCircle(x, y, rayon, color, nbSeg, 1, 2 * (float)M_PI, 0);
	}

	void drawCircle(int32_t x, float y, int32_t rayon, Color color)
	{
		if (rayon < 30)
			drawCircle(x, y, rayon, color, M_PI * rayon, 1, 2 * (float)M_PI, 0);
		else
			drawCircle(x, y, rayon, color, rayon, 1, 2 * (float)M_PI, 0);
	}

	void drawLineBegin(float lineWidth)
	{
		glLineWidth(lineWidth);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_LINES);
	}

	void drawLineEnd()
	{
		glEnd();
	}

	void drawLinePart(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color1, Color color2)
	{
		color1.bind();
		glVertex2f(x1 + .5f, y1 + .5f);
		color2.bind();
		glVertex2f(x2 + .5f, y2 + .5f);
	}

	void drawLinePart(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color1)
	{
		drawLinePart(x1, y1, x2, y2, color1, color1);
	}

	void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color1, Color color2, float lineWidth)
	{
		if (x1 != x2 && y1 != y2)
			glEnable(GL_LINE_SMOOTH);
		drawLineBegin(lineWidth);
		drawLinePart(x1, y1, x2, y2, color1, color2);
		drawLineEnd();
		if (x1 != x2 && y1 != y2)
			glDisable(GL_LINE_SMOOTH);
	}

	void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color1, Color color2)
	{
		drawLine(x1, y1, x2, y2, color1, color2, 1);
	}

	void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color, float lineWidth)
	{
		drawLine(x1, y1, x2, y2, color, color, lineWidth);
	}

	void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color)
	{
		drawLine(x1, y1, x2, y2, color, color, 1);
	}

	void drawPointBegin(float size)
	{
		glPointSize(size);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_POINTS);
	}

	void drawPointEnd()
	{
		glEnd();
	}

	void drawPointPart(int32_t x, int32_t y, Color color)
	{
		color.bind();
		glVertex2f(x + .5f, y + .5f);
	}

	void drawPoint(int32_t x, int32_t y, Color color, float size)
	{
		drawPointBegin(size);
		drawPointPart(x, y, color);
		drawPointEnd();
	}

	void drawPoint(float x, float y, Color color)
	{
		drawPoint(x, y, color, 1);
	}

}
