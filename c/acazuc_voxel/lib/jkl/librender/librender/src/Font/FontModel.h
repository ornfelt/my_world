#ifndef LIBRENDER_FONT_MODEL_H
# define LIBRENDER_FONT_MODEL_H

# include <ft2build.h>
# include FT_FREETYPE_H
# include <string>

namespace librender
{

	class Font;

	class FontModel
	{

	private:
		static FT_Library ftLib;
		FT_Face ftFace;
		void _load();

	public:
		FontModel(std::string filename);
		FontModel(const char *data, size_t len);
		~FontModel();
		Font *derive(uint32_t size, bool alphaTexture = true);
		bool setSize(uint32_t size);
		inline FT_Face &getFtFace() {return this->ftFace;};

	};

}

#endif
