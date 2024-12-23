#ifndef LIBRENDER_FONT_ATLAS_H
# define LIBRENDER_FONT_ATLAS_H

# include "./Glyph.h"
# include <unordered_map>
# include <cstdint>
# include <cstddef>
# include <vector>

namespace librender
{

	struct FontAtlasLine
	{
		uint32_t width;
		uint32_t height;
		uint32_t y;
	};

	class FontAtlas
	{

	private:
		std::unordered_map<uint32_t, Glyph> glyphs;
		std::vector<FontAtlasLine> lines;
		std::vector<uint8_t> data;
		uint32_t revision;
		uint32_t height;
		uint32_t width;
		uint32_t bpp;
		bool alphaTexture;

	public:
		FontAtlas(bool alphaTexture);
		~FontAtlas();
		bool findPlace(uint32_t width, uint32_t height, uint32_t *x, uint32_t *y);
		void grow();
		Glyph *addGlyph(uint32_t x, uint32_t y, uint32_t codepoint, Glyph glyph, const uint8_t *bitmap);
		Glyph *getGlyph(uint32_t codepoint);
		inline uint32_t getRevision() const {return this->revision;};
		inline uint32_t getHeight() const {return this->height;};
		inline uint32_t getWidth() const {return this->width;};
		inline bool isAlphaTexture() const {return this->alphaTexture;};
		inline const uint8_t *getData() const {return this->data.data();};
		inline size_t getDataSize() const {return this->data.size();};

	};

}

#endif
