#ifndef LIBRENDER_FONT_GLYPH_H
# define LIBRENDER_FONT_GLYPH_H

# include <cstdint>

namespace librender
{

	class Glyph
	{

	private:
		uint16_t advance;
		uint16_t height;
		uint16_t width;
		uint16_t texX;
		uint16_t texY;
		int16_t offsetX;
		int16_t offsetY;

	public:
		Glyph(uint16_t advance, uint16_t width, uint16_t height, int16_t offsetX, int16_t offsetY);
		inline void setAdvance(uint16_t advance) {this->advance = advance;};
		inline uint16_t getAdvance() {return this->advance;};
		inline void setHeight(uint16_t height) {this->height = height;};
		inline uint16_t getHeight() {return this->height;};
		inline void setWidth(uint16_t width) {this->width = width;};
		inline uint16_t getWidth() {return this->width;};
		inline void setTexX(uint16_t texX) {this->texX = texX;};
		inline uint16_t getTexX() {return this->texX;};
		inline void setTexY(uint16_t texY) {this->texY = texY;};
		inline uint16_t getTexY() {return this->texY;};
		inline void setOffsetX(int16_t offsetX) {this->offsetX = offsetX;};
		inline int16_t getOffsetX() {return this->offsetX;};
		inline void setOffsetY(int16_t offsetY) {this->offsetY = offsetY;};
		inline int16_t getOffsetY() {return this->offsetY;};

	};

}

#endif
