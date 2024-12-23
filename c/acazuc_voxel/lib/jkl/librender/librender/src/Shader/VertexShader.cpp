#include "VertexShader.h"
#include "ShaderException.h"
#include <cstring>

namespace librender
{

	VertexShader::VertexShader(const char *data)
	: Shader(GL_VERTEX_SHADER, data)
	{
	}

}
