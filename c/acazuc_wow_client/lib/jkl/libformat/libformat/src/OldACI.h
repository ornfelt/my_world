#ifndef LIB_FORMAT_OLD_ACI_H
# define LIB_FORMAT_OLD_ACI_H

# include <cstdint>
# include <string>

namespace libformat
{

	class OldACI
	{

		public:
			static bool read(std::string file, char *&data, uint32_t &width, uint32_t &height);
			static bool write(std::string file, char *data, uint32_t width, uint32_t height);

	};

}

#endif
