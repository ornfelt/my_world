#include "VertexBuffer.h"

namespace librender
{

	VertexBuffer::VertexBuffer()
	{
		glGenBuffers(1, &this->id);
	}

	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &this->id);
	}

	void VertexBuffer::setData(GLenum type, const void *data, GLsizei len, GLenum usage)
	{
		bind(type);
		glBufferData(type, len, data, usage);
	}
	
	void VertexBuffer::bind(GLenum type) const
	{
		glBindBuffer(type, this->id);
	}

}
