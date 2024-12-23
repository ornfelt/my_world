#ifndef LIBRENDER_FRAME_BUFFER_H
# define LIBRENDER_FRAME_BUFFER_H

# include "../Texture/Texture.h"
# include "../GL.h"

namespace librender
{

	class FrameBuffer
	{

	private:
		GLuint id;

	public:
		FrameBuffer();
		~FrameBuffer();
		void attachTexture(GLenum attachment, Texture &texture, GLint level);
		void draw(GLenum buf);
		void read(GLenum buf);
		void bind();
		inline GLuint getId() {return this->id;};

	};

}

#endif
