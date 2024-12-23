#ifndef LIBUNICODE_UNICODE_H
# define LIBUNICODE_UNICODE_H

namespace unicode
{

	template <class u32>
	bool is_codepoint_valid(u32 cp)
	{
		if (cp > 0x10FFFF)
			return false;
		if (cp >= 0xD800 && cp <= 0xDFFF)
			return false;
		return true;
	}

}

#endif
