#include "Program.h"
#include "ProgramException.h"
#include <cstring>

namespace librender
{

	Program::Program()
	{
		this->id = glCreateProgram();
	}

	Program::~Program()
	{
		glDeleteProgram(this->id);
	}

	void Program::attachShader(FragmentShader *fragmentShader)
	{
		glAttachShader(this->id, fragmentShader->getId());
	}

	void Program::detachShader(FragmentShader *fragmentShader)
	{
		glDetachShader(this->id, fragmentShader->getId());
	}

	void Program::attachShader(GeometryShader *geometryShader)
	{
		glAttachShader(this->id, geometryShader->getId());
	}

	void Program::detachShader(GeometryShader *geometryShader)
	{
		glDetachShader(this->id, geometryShader->getId());
	}

	void Program::attachShader(VertexShader *vertexShader)
	{
		glAttachShader(this->id, vertexShader->getId());
	}

	void Program::detachShader(VertexShader *vertexShader)
	{
		glDetachShader(this->id, vertexShader->getId());
	}

	void Program::link()
	{
		glLinkProgram(this->id);
		GLint result = GL_FALSE;
		int infoLogLength;
		glGetProgramiv(this->id, GL_LINK_STATUS, &result);
		glGetProgramiv(this->id, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (!result)
		{
			std::string error(infoLogLength, 0);
			glGetProgramInfoLog(this->id, infoLogLength, nullptr, const_cast<char*>(error.c_str()));
			throw ProgramException(error);
		}
	}

	ProgramLocation Program::getUniformLocation(const char *name)
	{
		return ProgramLocation(glGetUniformLocation(this->id, name));
	}

	ProgramLocation Program::getAttribLocation(const char *name)
	{
		return ProgramLocation(glGetAttribLocation(this->id, name));
	}

	void Program::use()
	{
		GLuint currentProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&currentProgram));
		if (currentProgram == this->id)
			return;
		glUseProgram(this->id);
	}

}
