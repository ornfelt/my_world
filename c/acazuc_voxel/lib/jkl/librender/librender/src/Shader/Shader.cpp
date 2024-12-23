#include "Shader.h"
#include "./ShaderException.h"

namespace librender
{

	Shader::Shader(GLenum type, const char *data)
	{
		this->id = glCreateShader(type);
		glShaderSource(this->id, 1, &data , nullptr);
		glCompileShader(this->id);
		GLint result = GL_FALSE;
		int infoLogLength;
		glGetShaderiv(this->id, GL_COMPILE_STATUS, &result);
		glGetShaderiv(this->id, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (!result)
		{
			std::string error(infoLogLength, 0);
			glGetShaderInfoLog(this->id, infoLogLength, nullptr, const_cast<char*>(error.c_str()));
			glDeleteShader(this->id);
			throw ShaderException(error);
		}
	}

	Shader::~Shader()
	{
		glDeleteShader(this->id);
	}

}
