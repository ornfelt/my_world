#ifndef LIBRENDER_PROGRAM_EXCEPTION_H
# define LIBRENDER_PROGRAM_EXCEPTION_H

# include <string>

namespace librender
{

	class ProgramException : public std::exception
	{

	private:
		std::string err;

	public:
		ProgramException(std::string str): err(str) {};
		~ProgramException() throw() {};
		const char *what() const throw() {return this->err.c_str();};

	};

}

#endif
