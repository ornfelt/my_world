#ifndef TEXT_TESSELLATOR_H
# define TEXT_TESSELLATOR_H

# include "../DrawableTessellator.h"
# include "../Font/Font.h"

namespace librender
{

	class TextTessellator : public DrawableTessellator
	{

	private:
		Font *font;

	public:
		TextTessellator();
		void draw();
		void addGlyph(Glyph *glyph, Vec2 pos, Vec4 color);
		void addChar(uint32_t character, Vec2 pos, Vec4 color);
		inline void setFont(Font *font) {this->font = font;};
		inline Font *getFont() {return this->font;};

	};

}

#endif
