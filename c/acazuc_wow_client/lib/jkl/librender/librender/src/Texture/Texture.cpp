#include "Texture.h"
#include "GL.h"

namespace librender
{

	Texture::Texture(void *data, uint32_t width, uint32_t height)
	{
		glGenTextures(1, &this->id);
		setFilter(TEXTURE_FILTER_LINEAR, TEXTURE_FILTER_LINEAR);
		setWrap(TEXTURE_WRAP_CLAMP_TO_BORDER, TEXTURE_WRAP_CLAMP_TO_BORDER);
		setData(data, width, height);
	}

	Texture::Texture()
	{
		glGenTextures(1, &this->id);
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &this->id);
	}

	void Texture::setData(void *data, uint32_t width, uint32_t height)
	{
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		this->width = width;
		this->height = height;
	}

	void Texture::setMinFilter(enum TextureFilterType type)
	{
		GLenum filter;
		switch (type)
		{
			case TEXTURE_FILTER_LINEAR:
				filter = GL_LINEAR;
				break;
			case TEXTURE_FILTER_NEAREST:
				filter = GL_NEAREST;
				break;
			default:
				return;
		}
		bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	}
	
	void Texture::setMagFilter(enum TextureFilterType type)
	{
		GLenum filter;
		switch (type)
		{
			case TEXTURE_FILTER_LINEAR:
				filter = GL_LINEAR;
				break;
			case TEXTURE_FILTER_NEAREST:
				filter = GL_NEAREST;
				break;
			default:
				return;
		}
		bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	}

	void Texture::setFilter(enum TextureFilterType minType, enum TextureFilterType magType)
	{
		setMinFilter(minType);
		setMagFilter(magType);
	}

	void Texture::setXWrap(enum TextureWrapType type)
	{
		GLenum wrap;
		switch (type)
		{
			case TEXTURE_WRAP_REPEAT:
				wrap = GL_REPEAT;
				break;
			case TEXTURE_WRAP_MIRRORED_REPEAT:
				wrap = GL_MIRRORED_REPEAT;
				break;
			case TEXTURE_WRAP_CLAMP_TO_EDGE:
				wrap = GL_CLAMP_TO_EDGE;
				break;
			case TEXTURE_WRAP_CLAMP_TO_BORDER:
				wrap = GL_CLAMP_TO_BORDER;
				break;
			default:
				return;
		}
		bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	}

	void Texture::setYWrap(enum TextureWrapType type)
	{
		GLenum wrap;
		switch (type)
		{
			case TEXTURE_WRAP_REPEAT:
				wrap = GL_REPEAT;
				break;
			case TEXTURE_WRAP_MIRRORED_REPEAT:
				wrap = GL_MIRRORED_REPEAT;
				break;
			case TEXTURE_WRAP_CLAMP_TO_EDGE:
				wrap = GL_CLAMP_TO_EDGE;
				break;
			case TEXTURE_WRAP_CLAMP_TO_BORDER:
				wrap = GL_CLAMP_TO_BORDER;
				break;
			default:
				return;
		}
		bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	}

	void Texture::setWrap(enum TextureWrapType xType, enum TextureWrapType yType)
	{
		setXWrap(xType);
		setYWrap(yType);
	}
	
	void Texture::bind()
	{
		/*GLuint currentTexture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, reinterpret_cast<GLint*>(&currentTexture));
		if (this->id == currentTexture)
			return;*/
		glBindTexture(GL_TEXTURE_2D, this->id);
	}

	void Texture::setWidth(uint32_t width)
	{
		this->width = width;
	}

	int32_t Texture::getWidth() const
	{
		return this->width;
	}

	void Texture::setHeight(uint32_t height)
	{
		this->height = height;
	}

	int32_t Texture::getHeight() const
	{
		return this->height;
	}

}
