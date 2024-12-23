#ifndef LIBAUDIO_EXCEPTION_H
# define LIBAUDIO_EXCEPTION_H

# include <stdexcept>
# include <string>

namespace libaudio
{

	class Exception : public std::exception
	{

	private:
		std::string err;

	public:
		Exception(std::string str): err(str) {};
		~Exception() throw() {};
		const char *what() const throw() {return this->err.c_str();};

	};

}

#endif
