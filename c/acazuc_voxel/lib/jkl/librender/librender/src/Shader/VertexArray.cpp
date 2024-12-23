#include "VertexArray.h"

namespace librender
{

	VertexArray::VertexArray()
	{
		glGenVertexArrays(1, &this->id);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &this->id);
	}

	void VertexArray::bind()
	{
		glBindVertexArray(this->id);
	}

	void VertexArray::unbind()
	{
		glBindVertexArray(0);
	}

}
