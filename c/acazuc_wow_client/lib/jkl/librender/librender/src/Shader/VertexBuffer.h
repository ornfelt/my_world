#ifndef LIBRENDER_VERTEX_BUFFER_H
# define LIBRENDER_VERTEX_BUFFER_H

# include "../GL.h"

namespace librender
{

	class VertexBuffer
	{

	private:
		GLuint id;

	public:
		VertexBuffer();
		~VertexBuffer();
		void setData(GLenum type, const void *data, GLsizei size, GLenum usage);
		void bind(GLenum type) const;

	};

}

#endif
