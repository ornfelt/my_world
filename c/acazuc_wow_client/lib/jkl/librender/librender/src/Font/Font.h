#ifndef LIBRENDER_FONT_H
# define LIBRENDER_FONT_H

# include "../Texture/Texture.h"
# include "./FontBase.h"
# include "./Glyph.h"
# include <cstdint>
# include <string>

namespace librender
{

	class Font : public FontBase
	{

	private:
		Texture texture;
		uint32_t lastAtlasRevision;
		uint32_t revision;

	public:
		Font(FontModel &parent, uint32_t size, bool alphaTexture = true);
		~Font();
		void bind();
		void glUpdate();
		inline Texture &getTexture() {return this->texture;};
		inline uint32_t getRevision() {return this->revision;};

	};

}

#endif
