#ifndef LIBRENDER_VERTEX_SHADER_H
# define LIBRENDER_VERTEX_SHADER_H

# include "./Shader.h"

namespace librender
{

	class VertexShader : public Shader
	{

	public:
		VertexShader(const char *data);

	};

}

#endif
