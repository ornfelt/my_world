#ifndef LIBRENDER_RENDER_BUFFER_H
# define LIBRENDER_RENDER_BUFFER_H

# include "../GL.h"

namespace librender
{

	class RenderBuffer
	{

	private:
		GLuint id;

	public:
		RenderBuffer();
		~RenderBuffer();
		void setStorage(GLenum format, GLsizei width, GLsizei height);
		void bind();
		inline GLuint getId() {return this->id;};

	};

}

#endif
