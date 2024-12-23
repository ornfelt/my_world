#include "GeometryShader.h"
#include "ShaderException.h"
#include <cstring>

namespace librender
{

	GeometryShader::GeometryShader(const char *data)
	: Shader(GL_GEOMETRY_SHADER_ARB, data)
	{
	}

}
