#ifndef LIBRENDER_DRAWABLE_BASE_H
# define LIBRENDER_DRAWABLE_BASE_H

# include "./Vec/Vec4.h"
# include "./Vec/Vec2.h"
# include <cstdint>
# include <vector>

namespace librender
{

	class DrawableBase
	{

	protected:
		std::vector<uint32_t> indices;
		std::vector<Vec2> texCoords;
		std::vector<Vec2> positions;
		std::vector<Vec4> colors;
		Vec2 scale;
		Vec2 pos;
		uint32_t verticesNumber;
		uint32_t indicesNumber;
		uint8_t updatesRequired;
		uint8_t buffers;
		inline virtual void updateTexCoords() {};
		inline virtual void updatePositions() {};
		inline virtual void updateIndices() {};
		inline virtual void updateColors() {};
		virtual void updateBuffers();

	public:
		DrawableBase();
		virtual ~DrawableBase() {};
		virtual void resize(uint32_t vertices, uint32_t indices);
		virtual void requireUpdates(uint8_t updates);
		inline std::vector<uint32_t> &getIndices() {return this->indices;};
		inline std::vector<Vec2> &getTexCoords() {return this->texCoords;};
		inline std::vector<Vec2> &getPositions() {return this->positions;};
		inline std::vector<Vec4> &getColors() {return this->colors;};
		inline uint32_t getVerticesNumber() {return this->verticesNumber;};
		inline uint32_t getIndicesNumber() {return this->indicesNumber;};
		inline virtual void setScaleX(float scaleX) {this->scale.x = scaleX;};
		inline float getScaleX() {return this->scale.x;};
		inline virtual void setScaleY(float scaleY) {this->scale.y = scaleY;};
		inline float getScaleY() {return this->scale.y;};
		inline void setScale(float scaleX, float scaleY) {setScaleX(scaleX);setScaleY(scaleY);};
		inline void setScale(float scale) {setScaleX(scale);setScaleY(scale);};
		inline Vec2 getScale() {return this->scale;};
		inline virtual void setX(float x) {this->pos.x = x;};
		inline float getX() const {return this->pos.x;};
		inline virtual void setY(float y) {this->pos.y = y;};
		inline float getY() const {return this->pos.y;};
		inline void setPos(float x, float y) {setX(x);setY(y);};

	};

}

#endif
