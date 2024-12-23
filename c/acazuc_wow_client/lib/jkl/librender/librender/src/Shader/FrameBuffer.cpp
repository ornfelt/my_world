#include "FrameBuffer.h"

namespace librender
{

	FrameBuffer::FrameBuffer()
	{
		glGenFramebuffers(1, &this->id);
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &this->id);
	}

	void FrameBuffer::attachTexture(GLenum attachment, Texture &texture, GLint level)
	{
		bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture.getId(), level);
	}

	void FrameBuffer::draw(GLenum buf)
	{
		glDrawBuffer(buf);
	}

	void FrameBuffer::read(GLenum buf)
	{
		glReadBuffer(buf);
	}

	void FrameBuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->id);
	}

}
