#include "RenderBuffer.h"

namespace librender
{

	RenderBuffer::RenderBuffer()
	{
		glGenRenderbuffers(1, &this->id);
	}

	RenderBuffer::~RenderBuffer()
	{
		glDeleteRenderbuffers(1, &this->id);
	}

	void RenderBuffer::setStorage(GLenum format, GLsizei width, GLsizei height)
	{
		bind();
		glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
	}

	void RenderBuffer::bind()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, this->id);
	}

}
