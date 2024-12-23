#include "FragmentShader.h"
#include "ShaderException.h"
#include <cstring>

namespace librender
{

	FragmentShader::FragmentShader(const char *data)
	: Shader(GL_FRAGMENT_SHADER, data)
	{
	}

}
