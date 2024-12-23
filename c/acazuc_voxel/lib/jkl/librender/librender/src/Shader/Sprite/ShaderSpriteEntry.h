#ifndef LIBRENDER_SHADER_SPRITE_ENTRY_H
# define LIBRENDER_SHADER_SPRITE_ENTRY_H

# include "../../Texture/Texture.h"
# include "../../Vec/Vec4.h"
# include "../../Vec/Vec2.h"
# include <vector>

namespace librender
{

	class ShaderSpriteEntry
	{

	protected:
		std::vector<Vec2> texCoords;
		std::vector<Vec2> vertexes;
		std::vector<Vec4> colors;
		Vec2 scale;
		Vec2 pos;
		uint32_t verticesNumber;
		uint8_t updatesRequired;

	public:
		ShaderSpriteEntry();
		virtual ~ShaderSpriteEntry();
		virtual void update();
		inline std::vector<Vec2> &getTexCoords() {return this->texCoords;};
		inline std::vector<Vec2> &getVertexes() {return this->vertexes;};
		inline std::vector<Vec4> &getColors() {return this->colors;};
		void setColor(Color color);
		void setTopColor(Color color);
		void setBotColor(Color color);
		void setLeftColor(Color color);
		void setRightColor(Color color);
		void setTopLeftColor(Color color);
		void setTopRightColor(Color color);
		void setBotLeftColor(Color color);
		void setBotRightColor(Color color);
		virtual Texture *getTexture() {return nullptr;};
		inline uint32_t getVerticesNumber() {return this->verticesNumber;};
		void setScaleX(float scaleX);
		inline float getScaleX() {return this->scale.x;};
		void setScaleY(float scaleY);
		inline float getScaleY() {return this->scale.y;};
		void setScale(float scaleX, float scaleY) {setScaleX(scaleX);setScaleY(scaleY);};
		void setScale(float scale) {setScaleX(scale);setScaleY(scale);};
		void setTexTopLeft(Vec2 pos);
		void setTexTopRight(Vec2 pos);
		void setTexBottomLeft(Vec2 pos);
		void setTexBottomRight(Vec2 pos);
		void setTex(Vec2 topLeft, Vec2 topRight, Vec2 bottomLeft, Vec2 bottomRight);
		void setTexX(float texX);
		void setTexY(float texY);
		void setTexPos(float texX, float texY);
		void setTexWidth(float texWidth);
		void setTexHeight(float texHeight);
		void setTexSize(float texWidth, float texHeight);
		virtual void setWidth(float width);
		virtual void setHeight(float height);
		void setSize(float width, float height);
		inline virtual void setX(float x) {this->pos.x = x;};
		inline float getX() {return this->pos.x;};
		inline virtual void setY(float y) {this->pos.y = y;};
		inline float getY() {return this->pos.y;};
		inline void setPos(float x, float y) {setX(x);setY(y);};
		int32_t getTextureWidth();
		int32_t getTextureHeight();

	};

}

#endif
