#ifndef LIBRENDER_SPRITE_BASE_H
# define LIBRENDER_SPRITE_BASE_H

# include "../Texture/Texture.h"
# include "../DrawableBase.h"

namespace librender
{

	class SpriteBase : public virtual DrawableBase
	{

	private:
		Vec2 size;
		void updatePositions();

	public:
		SpriteBase();
		void setColor(Color color);
		void setTopColor(Color color);
		void setBotColor(Color color);
		void setLeftColor(Color color);
		void setRightColor(Color color);
		void setTopLeftColor(Color color);
		void setTopRightColor(Color color);
		void setBotLeftColor(Color color);
		void setBotRightColor(Color color);
		virtual Texture *getTexture() const {return nullptr;};
		void setTexTopLeft(Vec2 pos);
		Vec2 getTexTopLeft();
		void setTexTopRight(Vec2 pos);
		Vec2 getTexTopRight();
		void setTexBottomLeft(Vec2 pos);
		Vec2 getTexBottomLeft();
		void setTexBottomRight(Vec2 pos);
		Vec2 getTexBottomRight();
		void setTex(Vec2 topLeft, Vec2 topRight, Vec2 bottomLeft, Vec2 bottomRight);
		void setTexX(float texX);
		void setTexY(float texY);
		void setTexPos(float texX, float texY);
		void setTexWidth(float texWidth);
		void setTexHeight(float texHeight);
		void setTexSize(float texWidth, float texHeight);
		void setWidth(float width);
		float getWidth() const;
		void setHeight(float height);
		float getHeight() const;
		void setSize(Vec2 size);
		inline void setSize(float width, float height) {setSize(Vec2(width, height));};
		int32_t getTextureWidth() const;
		int32_t getTextureHeight() const;

	};

}

#endif
