#ifndef LIBRENDER_SHADER_EXCEPTION_H
# define LIBRENDER_SHADER_EXCEPTION_H

# include <string>

namespace librender
{

	class ShaderException : public std::exception
	{

	private:
		std::string err;

	public:
		ShaderException(std::string str): err(str) {};
		~ShaderException() throw() {};
		const char *what() const throw() {return this->err.c_str();};

	};

}

#endif
