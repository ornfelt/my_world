#include "ShaderSpriteEntry.h"
#include "./ShaderSpriteUpdate.h"
#include <cstring>

namespace librender
{

	ShaderSpriteEntry::ShaderSpriteEntry()
	: texCoords(4)
	, vertexes(4)
	, colors(4)
	, scale(1)
	, pos(0)
	, verticesNumber(4)
	, updatesRequired(0)
	{
		texCoords[0] = Vec2(0, 0);
		texCoords[1] = Vec2(1, 0);
		texCoords[2] = Vec2(1, 1);
		texCoords[3] = Vec2(0, 1);
		for (uint8_t i = 0; i < 4; ++i)
			colors[i] = Vec4(1);
		for (uint8_t i = 0; i < 4; ++i)
			vertexes[i] = Vec2(0);
		this->updatesRequired = SHADER_SPRITE_UPDATE_TEX_COORDS | SHADER_SPRITE_UPDATE_VERTEXES | SHADER_SPRITE_UPDATE_COLORS;
	}

	ShaderSpriteEntry::~ShaderSpriteEntry()
	{
	}

	void ShaderSpriteEntry::update()
	{
		this->updatesRequired = 0;
	}

	void ShaderSpriteEntry::setColor(Color color)
	{
		setTopLeftColor(color);
		setTopRightColor(color);
		setBotLeftColor(color);
		setBotRightColor(color);
	}

	void ShaderSpriteEntry::setTopColor(Color color)
	{
		setTopLeftColor(color);
		setTopRightColor(color);
	}

	void ShaderSpriteEntry::setBotColor(Color color)
	{
		setBotLeftColor(color);
		setBotRightColor(color);
	}

	void ShaderSpriteEntry::setLeftColor(Color color)
	{
		setTopLeftColor(color);
		setBotLeftColor(color);
	}
	
	void ShaderSpriteEntry::setRightColor(Color color)
	{
		setTopRightColor(color);
		setBotRightColor(color);
	}

	void ShaderSpriteEntry::setTopLeftColor(Color color)
	{
		std::memcpy(&this->colors[0], &color, sizeof(*this->colors.data()));
		this->updatesRequired |= SHADER_SPRITE_UPDATE_COLORS;
	}

	void ShaderSpriteEntry::setTopRightColor(Color color)
	{
		std::memcpy(&this->colors[1], &color, sizeof(*this->colors.data()));
		this->updatesRequired |= SHADER_SPRITE_UPDATE_COLORS;
	}

	void ShaderSpriteEntry::setBotLeftColor(Color color)
	{
		std::memcpy(&this->colors[2], &color, sizeof(*this->colors.data()));
		this->updatesRequired |= SHADER_SPRITE_UPDATE_COLORS;
	}

	void ShaderSpriteEntry::setBotRightColor(Color color)
	{
		std::memcpy(&this->colors[3], &color, sizeof(*this->colors.data()));
		this->updatesRequired |= SHADER_SPRITE_UPDATE_COLORS;
	}

	void ShaderSpriteEntry::setScaleX(float scaleX)
	{
		if (this->scale.x == scaleX)
			return;
		this->scale.y = scaleX;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_VERTEXES;
	}

	void ShaderSpriteEntry::setScaleY(float scaleY)
	{
		if (this->scale.x == scaleY)
			return;
		this->scale.y = scaleY;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_VERTEXES;
	}

	void ShaderSpriteEntry::setTexTopLeft(Vec2 pos)
	{
		this->texCoords[0] = pos;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
	}

	void ShaderSpriteEntry::setTexTopRight(Vec2 pos)
	{
		this->texCoords[1] = pos;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
	}

	void ShaderSpriteEntry::setTexBottomLeft(Vec2 pos)
	{
		this->texCoords[3] = pos;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
	}

	void ShaderSpriteEntry::setTexBottomRight(Vec2 pos)
	{
		this->texCoords[2] = pos;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
	}

	void ShaderSpriteEntry::setTex(Vec2 topLeft, Vec2 topRight, Vec2 bottomLeft, Vec2 bottomRight)
	{
		this->texCoords[0] = topLeft;
		this->texCoords[1] = topRight;
		this->texCoords[2] = bottomRight;
		this->texCoords[3] = bottomLeft;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
	}

	void ShaderSpriteEntry::setTexX(float texX)
	{
		float delta = texX - this->texCoords[0].x;
		if (!delta)
			return;
		this->texCoords[0].x = texX;
		this->texCoords[1].x += delta;
		this->texCoords[2].x += delta;
		this->texCoords[3].x = texX;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
	}

	void ShaderSpriteEntry::setTexY(float texY)
	{
		float delta = texY - this->texCoords[0].y;
		if (!delta)
			return;
		this->texCoords[0].y = texY;
		this->texCoords[1].y = texY;
		this->texCoords[2].y += delta;
		this->texCoords[3].y += delta;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
	}

	void ShaderSpriteEntry::setTexPos(float texX, float texY)
	{
		setTexX(texX);
		setTexY(texY);
	}

	void ShaderSpriteEntry::setTexWidth(float texWidth)
	{
		float delta = texWidth - (this->texCoords[1].x - this->texCoords[0].x);
		if (!delta)
			return;
		this->texCoords[1].x = texWidth + this->texCoords[0].x;
		this->texCoords[2].x = texWidth + this->texCoords[0].x;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
	}

	void ShaderSpriteEntry::setTexHeight(float texHeight)
	{
		float delta = texHeight - (this->texCoords[3].y - this->texCoords[0].y);
		if (!delta)
			return;
		this->texCoords[2].y = texHeight + this->texCoords[0].y;
		this->texCoords[3].y = texHeight + this->texCoords[0].y;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_TEX_COORDS;
	}

	void ShaderSpriteEntry::setTexSize(float texWidth, float texHeight)
	{
		setTexWidth(texWidth);
		setTexHeight(texHeight);
	}

	void ShaderSpriteEntry::setWidth(float width)
	{
		float delta = width - (this->vertexes[1].x - this->vertexes[0].x);
		if (!delta)
			return;
		this->vertexes[1].x = width;
		this->vertexes[2].x = width;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_VERTEXES;
	}

	void ShaderSpriteEntry::setHeight(float height)
	{
		float delta = height - (this->vertexes[3].y - this->vertexes[0].y);
		if (!delta)
			return;
		this->vertexes[2].y = height;
		this->vertexes[3].y = height;
		this->updatesRequired |= SHADER_SPRITE_UPDATE_VERTEXES;
	}

	void ShaderSpriteEntry::setSize(float width, float height)
	{
		setWidth(width);
		setHeight(height);
	}

	int32_t ShaderSpriteEntry::getTextureWidth()
	{
		if (!getTexture())
			return 0;
		return getTexture()->getWidth();
	}

	int32_t ShaderSpriteEntry::getTextureHeight()
	{
		if (!getTexture())
			return 0;
		return getTexture()->getHeight();
	}

}
