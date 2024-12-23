#ifndef TEXTURE_H
# define TEXTURE_H

# include "../Color.h"
# include <cstdint>

namespace librender
{

	enum TextureFilterType
	{
		TEXTURE_FILTER_LINEAR,
		TEXTURE_FILTER_NEAREST
	};

	enum TextureWrapType
	{
		TEXTURE_WRAP_REPEAT,
		TEXTURE_WRAP_MIRRORED_REPEAT,
		TEXTURE_WRAP_CLAMP_TO_EDGE,
		TEXTURE_WRAP_CLAMP_TO_BORDER
	};

	class Texture
	{

	private:
		uint32_t width;
		uint32_t height;
		uint32_t id;

	public:
		Texture(void *data, uint32_t width, uint32_t height);
		Texture();
		~Texture();
		void bind();
		void setData(void *data, uint32_t width, uint32_t height);
		void setMinFilter(enum TextureFilterType type);
		void setMagFilter(enum TextureFilterType type);
		void setFilter(enum TextureFilterType minType, enum TextureFilterType magType);
		void setXWrap(enum TextureWrapType type);
		void setYWrap(enum TextureWrapType type);
		void setWrap(enum TextureWrapType xType, enum TextureWrapType yType);
		void setWidth(uint32_t width);
		int32_t getWidth() const;
		void setHeight(uint32_t height);
		int32_t getHeight() const;
		inline uint32_t getId() {return this->id;};

	};

}

#endif
