#ifndef LIBUNICODE_UTF8_H
# define LIBUNICODE_UTF8_H

# include "unicode.h"
# include <stdexcept>

namespace utf8
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
		if (!(*it & 0x80))
			return 1;
		if ((*it & 0xF8) == 0xF0)
			return 4;
		if ((*it & 0xF0) == 0xE0)
			return 3;
		if ((*it & 0xE0) == 0xC0)
			return 2;
		return 0;
	}

	template <class byte_iterator>
	void encode1(byte_iterator &it, uint32_t cp)
	{
		*(it++) = cp & 0x7F;
	}

	template <class byte_iterator>
	bool check1(byte_iterator &it)
	{
		(void)it;
		return true;
	}

	template <class byte_iterator>
	uint32_t decode1(byte_iterator &it)
	{
		return *(it++);
	}

	template <class byte_iterator>
	void encode2(byte_iterator &it, uint32_t cp)
	{
		*(it++) = 0xC0 | ((cp >> 0x6) & 0x1F);
		*(it++) = 0x80 | ((cp >> 0x0) & 0x3F);
	}

	template <class byte_iterator>
	bool check2(byte_iterator &it)
	{
		if ((it[0] & 0x1F) < 2)
			return false;
		if ((it[1] & 0xC0) != 0x80)
			return false;
		return true;
	}

	template <class byte_iterator>
	uint32_t decode2(byte_iterator &it)
	{
		uint32_t cp = static_cast<uint32_t>(*(it++) & 0x1F) << 6;
		cp |= *(it++) & 0x3F;
		return cp;
	}

	template <class byte_iterator>
	void encode3(byte_iterator &it, uint32_t cp)
	{
		*(it++) = 0xE0 | ((cp >> 0xC) & 0x0F);
		*(it++) = 0x80 | ((cp >> 0x6) & 0x3F);
		*(it++) = 0x80 | ((cp >> 0x0) & 0x3F);
	}

	template <class byte_iterator>
	bool check3(byte_iterator &it)
	{
		if (!(it[0] & 0xF) && !(it[1] & 0x20))
			return false;
		if ((it[1] & 0xC0) != 0x80)
			return false;
		if ((it[2] & 0xC0) != 0x80)
			return false;
		return true;
	}

	template <class byte_iterator>
	uint32_t decode3(byte_iterator &it)
	{
		uint32_t cp = static_cast<uint32_t>(*(it++) & 0xF) << 12;
		cp |= static_cast<uint32_t>(*(it++) & 0x3F) << 6;
		cp |= *(it++) & 0x3F;
		return cp;
	}

	template <class byte_iterator>
	void encode4(byte_iterator &it, uint32_t cp)
	{
		*(it++) = 0xF0 | ((cp >> 0xC) & 0x07);
		*(it++) = 0x80 | ((cp >> 0x6) & 0x3F);
		*(it++) = 0x80 | ((cp >> 0x6) & 0x3F);
		*(it++) = 0x80 | ((cp >> 0x0) & 0x3F);
	}

	template <class byte_iterator>
	bool check4(byte_iterator &it)
	{
		if (!(it[0] & 0x7) && !(it[1] & 0x30))
			return false;
		if ((it[1] & 0xC0) != 0x80)
			return false;
		if ((it[2] & 0xC0) != 0x80)
			return false;
		if ((it[3] & 0xC0) != 0x80)
			return false;
		return true;
	}

	template <class byte_iterator>
	uint32_t decode4(byte_iterator &it)
	{
		uint32_t cp = static_cast<uint32_t>(*(it++) & 0x7) << 18;
		cp |= static_cast<uint32_t>(*(it++) & 0x3F) << 12;
		cp |= static_cast<uint32_t>(*(it++) & 0x3F) << 6;
		cp |= *(it++) & 0x3F;
		return cp;
	}

	template <class byte_iterator>
	void encode(byte_iterator &it, uint32_t cp)
	{
		if (cp < 0x80)
			encode1(it, cp);
		else if (cp < 0x800)
			encode2(it, cp);
		else if (cp < 0x10000)
			encode3(it, cp);
		else if (cp < 0x10FFFF)
			encode4(it, cp);
		else
			throw invalid_codepoint();
	}

	template <class byte_iterator>
	uint32_t check(byte_iterator &it)
	{
		switch (codepoint_length(it))
		{
			case 1:
				return check1(it);
			case 2:
				return check2(it);
			case 3:
				return check3(it);
			case 4:
				return check4(it);
		}
		throw invalid_sequence();
		return 0;
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
			case 3:
				return decode3(it);
			case 4:
				return decode4(it);
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
				if (!check1(it))
					throw invalid_sequence();
				return decode1(it);
			}
			case 2:
			{
				if (end - it < 2)
					throw not_enough_room();
				if (!check2(it))
					throw invalid_sequence();
				return decode2(it);
			}
			case 3:
			{
				if (end - it < 3)
					throw not_enough_room();
				if (!check3(it))
					throw invalid_sequence();
				return decode3(it);
			}
			case 4:
			{
				if (end - it < 4)
					throw not_enough_room();
				if (!check4(it))
					throw invalid_sequence();
				return decode4(it);
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
		while (it > begin)
		{
			--it;
			switch (codepoint_length(it))
			{
				case 1:
				{
					if (org - it != 1)
						throw invalid_sequence();
					if (!check1(it))
						throw invalid_sequence();
					byte_iterator tmp(it);
					return decode1(tmp);
				}
				case 2:
				{
					if (org - it != 2)
						throw invalid_sequence();
					if (!check2(it))
						throw invalid_sequence();
					byte_iterator tmp(it);
					return decode2(tmp);
				}
				case 3:
				{
					if (org - it != 3)
						throw invalid_sequence();
					if (!check3(it))
						throw invalid_sequence();
					byte_iterator tmp(it);
					return decode3(tmp);
				}
				case 4:
				{
					if (org - it != 4)
						throw invalid_sequence();
					if (!check4(it))
						throw invalid_sequence();
					byte_iterator tmp(it);
					return decode4(tmp);
				}
			}
		}
		throw not_enough_room();
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
				++it;
				break;
			case 2:
				if (end - it < 2)
					throw not_enough_room();
				if (!check2(it))
					throw invalid_sequence();
				it += 2;
				break;
			case 3:
				if (end - it < 3)
					throw not_enough_room();
				if (!check3(it))
					throw invalid_sequence();
				it += 3;
				break;
			case 4:
				if (end - it < 4)
					throw not_enough_room();
				if (!check4(it))
					throw invalid_sequence();
				it += 4;
				break;
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
					++it;
					if (!check1(it))
						return it;
					break;
				case 2:
					if (end - it < 2)
						return it;
					if (!check2(it))
						return it;
					it += 2;
					break;
				case 3:
					if (end - it < 3)
						return it;
					if (!check3(it))
						return it;
					it += 3;
					break;
				case 4:
					if (end - it < 4)
						return it;
					if (!check4(it))
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
	bool is_valid(byte_iterator it, byte_iterator end)
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
		while (it != end)
		{
			switch (codepoint_length(it))
			{
				case 1:
					*(out++) = *(it++);
					break;
				case 2:
					if (end - it < 2)
						throw not_enough_room();
					if (!check2(it))
						goto invalid;
					{
						byte_iterator tmp(it);
						if (!unicode::is_codepoint_valid(decode2(tmp)))
							goto invalid;
					}
					*(out++) = *(it++);
					*(out++) = *(it++);
					break;
				case 3:
					if (end - it < 3)
						throw not_enough_room();
					if (!check3(it))
						goto invalid;
					{
						byte_iterator tmp(it);
						if (!unicode::is_codepoint_valid(decode3(tmp)))
							goto invalid;
					}
					*(out++) = *(it++);
					*(out++) = *(it++);
					*(out++) = *(it++);
					break;
				case 4:
					if (end - it < 4)
						throw not_enough_room();
					if (!check4(it))
						goto invalid;
					{
						byte_iterator tmp(it);
						if (!unicode::is_codepoint_valid(decode4(tmp)))
							goto invalid;
					}
					*(out++) = *(it++);
					*(out++) = *(it++);
					*(out++) = *(it++);
					*(out++) = *(it++);
					break;
				default:
					out = append(replacement, out);
					++it;
					break;
				invalid:
					out = append(replacement, out);
					while (++it != end && (*it & 0xC0) == 0x80)
						;
					break;
			}
		}
	}

};

#endif
