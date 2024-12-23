#ifndef LIBUNICODE_UTF16_H
# define LIBUNICODE_UTF16_H

# include "unicode.h"
# include <stdexcept>

namespace utf16
{

	struct exception : public std::exception
	{
	};

	struct invalid_sequence : public exception
	{
	};

	struct not_enough_room : public exception
	{
	};

	struct invalid_codepoint : public exception
	{
	};

	template <class byte_iterator>
	uint32_t codepoint_length(byte_iterator it)
	{
		if ((*it & 0xF8) == 0xD8)
			return 2;
		return 1;
	}

	template <class byte_iterator>
	void encode1(byte_iterator &it, uint32_t cp)
	{
		*(it++) = cp & 0xff;
		*(it++) = (cp >> 0x8) >> 8;
	}

	template <class byte_iterator>
	uint32_t decode1(byte_iterator &it)
	{
		uint32_t cp = static_cast<uint32_t>(*(it++)) << 8;
		cp |= static_cast<uint32_t>(*(it++));
		return cp;
	}

	template <class byte_iterator>
	void encode2(byte_iterator &it, uint32_t cp)
	{
		*(it++) = 0xD8 | ((cp >> 20) & 0x3);
		*(it++) = cp >> 10;
		*(it++) = 0xDC | ((cp >> 8) & 0x3);
		*(it++) = cp & 0xff;
	}

	template <class byte_iterator>
	uint32_t decode2(byte_iterator &it)
	{
		if ((it[2] & 0xF8) != 0xD8)
			throw invalid_sequence();
		uint32_t cp = static_cast<uint32_t>(*(it++) & 0x3) << 18;
		cp |= static_cast<uint32_t>(*(it++)) << 10;
		cp |= static_cast<uint32_t>(*(it++) & 0x3) << 8;
		cp |= *(it++);
		return cp;
	}

	template <class byte_iterator>
	void encode(byte_iterator &it, uint32_t cp)
	{
		if (cp < 0x10000)
			encode1(it, cp);
		if (cp < 0x100000)
			encode2(it, cp);
		else
			throw invalid_codepoint();
	}

	template <class byte_iterator>
	uint32_t decode(byte_iterator &it)
	{
		switch (codepoint_length(it))
		{
			case 1:
				return decode1(it);
			case 2:
				return decode2(it);
		}
		throw invalid_sequence();
		return 0;
	}

	template <class byte_iterator>
	uint32_t next(byte_iterator &it, byte_iterator end)
	{
		switch (codepoint_length(it))
		{
			case 1:
			{
				if (end - it < 2)
					throw not_enough_room();
				return decode1(it);
			}
			case 2:
			{
				if (end - it < 4)
					throw not_enough_room();
				return decode2(it);
			}
		}
		throw invalid_sequence();
		return 0;
	}

	template <class byte_iterator>
	uint32_t peek_next(byte_iterator it, byte_iterator end)
	{
		return next(it, end);
	}

	template <class byte_iterator>
	uint32_t prior(byte_iterator &it, byte_iterator begin)
	{
		byte_iterator org = it;
		while (it != begin)
		{
			--it;
			if (it == begin)
				throw not_enough_room();
			--it;
			switch (codepoint_length(it))
			{
				case 1:
				{
					if (org - it != 2)
						throw invalid_sequence();
					byte_iterator tmp(it);
					return decode1(tmp);
				}
				case 2:
				{
					if (org - it != 4)
						throw invalid_sequence();
					byte_iterator tmp(it);
					return decode2(tmp);
				}
			}
		}
		throw not_enough_room();
		return 0;
	}

	template <class byte_iterator>
	uint32_t peek_prior(byte_iterator it, byte_iterator begin)
	{
		return prior(it, begin);
	}

	template <class byte_iterator>
	void advance_step(byte_iterator &it, byte_iterator end)
	{
		switch (codepoint_length(it))
		{
			case 1:
				if (end - it < 2)
					throw not_enough_room();
				it += 2;
				return;
			case 2:
				if (end - it < 4)
					throw not_enough_room();
				it += 4;
				return;
			default:
				throw invalid_sequence();
		}
	}

	template <class byte_iterator>
	void advance(byte_iterator &it, size_t len, byte_iterator end)
	{
		for (size_t i = 0; i < len; ++i)
			advance_step(it, end);
	}

	template <class byte_iterator>
	size_t distance(byte_iterator it, byte_iterator end)
	{
		size_t i = 0;
		while (it != end)
		{
			advance_step(it, end);
			++i;
		}
		return i;
	}

	template <class byte_iterator>
	byte_iterator find_invalid(byte_iterator it, byte_iterator end)
	{
		while (it != end)
		{
			switch (codepoint_length(it))
			{
				case 1:
					if (end - it < 2)
						return it;
					it += 2;
					break;
				case 2:
					if (end - it < 4)
						return it;
					it += 4;
					break;
				default:
					return it;
			}
		}
		return it;
	}

	template <class byte_iterator>
	bool is_valid(byte_iterator t, byte_iterator end)
	{
		return find_invalid(it, end) == end;
	}

	template <class byte_iterator>
	byte_iterator append(uint32_t cp, byte_iterator out)
	{
		if (!unicode::is_codepoint_valid(cp))
			throw invalid_codepoint();
		encode(out, cp);
		return out;
	}

	template <class byte_iterator, class output_iterator>
	void replace_invalid(byte_iterator it, byte_iterator end, output_iterator out, uint32_t replacement = 0xFFFD)
	{
		uint32_t replacement_len = codepoint_length(it);
		while (it != end)
		{
			switch (codepoint_length(it))
			{
				case 1:
					if (end - it < 2)
						throw invalid_sequence();
					try
					{
						byte_iterator tmp(it);
						uint32_t cp = decode1(tmp);
						if (!unicode::is_codepoint_valid(cp))
							goto invalid;
						*(out++) = *(it++);
						*(out++) = *(it++);
					}
					catch (std::exception &e)
					{
						goto invalid;
					}
					break;
				case 2:
					if (end - it < 4)
						throw invalid_sequence();
					try
					{
						byte_iterator tmp(it);
						uint32_t cp = decode2(tmp);
						if (!unicode::is_codepoint_valid(cp))
							goto invalid;
						*(out++) = *(it++);
						*(out++) = *(it++);
						*(out++) = *(it++);
						*(out++) = *(it++);
					}
					catch (std::exception &e)
					{
						goto invalid;
					}
					break;
				default:
				invalid:
					out = append(replacement, out);
					++it;
					break;
			}
		}
	}

}

#endif
