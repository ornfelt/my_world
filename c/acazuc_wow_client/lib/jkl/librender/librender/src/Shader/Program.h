#ifndef LIBRENDER_PROGRAM_H
# define LIBRENDER_PROGRAM_H

# include "./ProgramLocation.h"
# include "./FragmentShader.h"
# include "./GeometryShader.h"
# include "./VertexShader.h"
# include "../GL.h"

namespace librender
{

	class Program
	{

	private:
		GLuint id;

	public:
		Program();
		~Program();
		void attachShader(FragmentShader *fragmentShader);
		void detachShader(FragmentShader *fragmentShader);
		void attachShader(GeometryShader *geometryShader);
		void detachShader(GeometryShader *geometryShader);
		void attachShader(VertexShader *vertexShader);
		void detachShader(VertexShader *vertexShader);
		void link();
		ProgramLocation getUniformLocation(const char *name);
		ProgramLocation getAttribLocation(const char *name);
		void use();
		inline GLuint getId() {return this->id;};

	};

}

#endif
