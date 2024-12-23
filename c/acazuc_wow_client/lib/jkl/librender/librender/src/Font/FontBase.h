#ifndef LIBRENDER_FONT_BASE_H
# define LIBRENDER_FONT_BASE_H

# include "./FontModel.h"
# include "./FontAtlas.h"
# include "./Glyph.h"

namespace librender
{

	class FontBase
	{

	protected:
		FontModel &parent;
		FontAtlas atlas;
		uint32_t height;
		uint32_t size;
		Glyph *loadGlyph(uint32_t codepoint);
		void texCoords(int32_t texX, int32_t texY, int32_t texWidth, int32_t texHeight, float *texCoords);

	public:
		FontBase(FontModel &parent, uint32_t size, bool alphaTexture = true);
		~FontBase();
		Glyph *getGlyph(uint32_t codepoint);
		void charTexCoords(uint32_t codepoint, float *texCoords);
		void glyphTexCoords(Glyph *glyph, float *texCoords);
		int32_t getWidth(const std::string &text);
		int32_t getHeight(const std::string &text);
		inline int32_t getLineHeight() {return this->height;};

	};

}

#endif
