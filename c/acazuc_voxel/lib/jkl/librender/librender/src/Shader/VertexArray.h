#ifndef LIBRENDER_VERTEX_ARRAY_H
# define LIBRENDER_VERTEX_ARRAY_H

# include "../GL.h"

namespace librender
{

	class VertexArray
	{

	private:
		GLuint id;

	public:
		VertexArray();
		~VertexArray();
		void bind();
		void unbind();

	};

}

#endif
