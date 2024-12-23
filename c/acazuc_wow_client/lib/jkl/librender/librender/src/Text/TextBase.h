#ifndef LIBRENDER_TEXT_BASE_H
# define LIBRENDER_TEXT_BASE_H

# include "../DrawableBase.h"
# include "../Font/Font.h"
# include <string>

namespace librender
{

	class TextBase : public virtual DrawableBase
	{

	protected:
		std::string text;
		Color shadowColor;
		Color topColor;
		Color botColor;
		int16_t shadowSize;
		int32_t lineHeight;
		int32_t maxWidth;
		int32_t shadowX;
		int32_t shadowY;
		int32_t height;
		int32_t width;
		size_t fontRevision;
		size_t charsNumber;
		bool mustCalcHeight;
		bool mustCalcWidth;
		void updateTexCoords();
		void updatePositions();
		void updateIndices();
		void updateColors();
		size_t getShadowLen();
		void updateBuffers();

	public:
		TextBase();
		void setText(const std::string &text);
		inline const std::string &getText() const {return this->text;};
		inline void setShadow(int32_t x, int32_t y, int32_t size, Color color) {setShadowPos(x, y);setShadowSize(size);setShadowColor(color);};
		void setShadowColor(Color color);
		inline Color getShadowColor() const {return this->shadowColor;};
		void setTopColor(Color color);
		void setBotColor(Color color);
		void setColor(Color color);
		inline virtual Font *getFont() {return nullptr;};
		inline void setLineHeight(uint32_t lineHeight) {this->lineHeight = lineHeight;};
		int32_t getLineHeight();
		void setShadowSize(int16_t shadowSize);
		inline int16_t getShadowSize() {return this->shadowSize;};
		inline void setShadowPos(int32_t x, int32_t y) {setShadowX(x);setShadowY(y);};
		void setShadowX(int32_t shadowX);
		inline int32_t getShadowX() {return this->shadowX;};
		void setShadowY(int32_t shadowY);
		inline int32_t getShadowY() {return this->shadowY;};
		void setMaxWidth(int32_t width);
		inline int32_t getMaxWidth() {return this->maxWidth;};
		inline void recalcWidth() {this->mustCalcWidth = true;};
		inline void recalcHeight() {this->mustCalcHeight = true;};
		int32_t getWidth();
		int32_t getHeight();

	};

}

#endif
