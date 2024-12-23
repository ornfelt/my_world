#ifndef LIBSQL_EXCEPTION_H
# define LIBSQL_EXCEPTION_H

# include <stdexcept>
# include <string>

namespace libsql
{

	class Exception : public std::exception
	{

	private:
		std::string str;
		unsigned int err;

	public:
		Exception(unsigned int err, std::string str) : str(str), err(err) {};
		~Exception() throw() {};
		unsigned int getErrno() {return this->err;};
		const char *what() const throw() {return this->str.c_str();};

	};

}

#endif
