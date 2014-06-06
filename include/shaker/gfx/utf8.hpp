#ifndef __GFX_UTF8_HPP__
#define __GFX_UTF8_HPP__

#include <string>
#include <stdint.h>

namespace std
{
	typedef basic_string<uint32_t> ustring;
}

namespace utf8
{
	using std::ustring;
	using std::string;

	static const uint32_t UCS_REPLACEMENT = 0xFFFD;

	namespace impl
	{
		template<typename octet_type>
		inline uint8_t mask8(octet_type oc)
		{
			return static_cast<uint8_t>(0xff & oc);
		}

		template <typename octet_iterator>
		inline typename std::iterator_traits<octet_iterator>::difference_type
			sequence_length(octet_iterator lead_it)
		{
				uint8_t lead = mask8(*lead_it);
				if (lead < 0x80)
					return 1;
				else if ((lead >> 5) == 0x6)
					return 2;
				else if ((lead >> 4) == 0xe)
					return 3;
				else if ((lead >> 3) == 0x1e)
					return 4;
				else
					return 0;
			}

#define UTF8_TAKE_FIRST() if (it == end) break; code_point = mask8(*it)
#define UTF8_MOVE() ++it; if (it == end) break
#define UTF8_SHIFT(cp_shift, cp_mask) do {  code_point <<= (cp_shift); code_point &= (cp_mask); } while (0)
#define UTF8_ADD(it_shift, it_mask) do {  code_point += (mask8(*it) << it_shift) & it_mask; } while (0)
#define UTF8_ADD_MASK(it_mask) do {  code_point += mask8(*it) & it_mask; } while (0)

		template <typename octet_iterator>
		bool next(octet_iterator& it, octet_iterator end, uint32_t& code_point)
		{
			auto save = it;
			auto length = sequence_length(it);

			switch (length) {
			case 1:
				UTF8_TAKE_FIRST();
				++it;
				return true;
			case 2:
				UTF8_TAKE_FIRST();
				UTF8_MOVE();
				UTF8_SHIFT(6, 0x7FF);
				UTF8_ADD_MASK(0x3F);
				++it;
				return true;
			case 3:
				UTF8_TAKE_FIRST();
				UTF8_MOVE();
				UTF8_SHIFT(12, 0xFFFF);
				UTF8_ADD(6, 0xFFF);
				UTF8_MOVE();
				UTF8_ADD_MASK(0x3F);
				++it;
				return true;
			case 4:
				UTF8_TAKE_FIRST();
				UTF8_MOVE();
				UTF8_SHIFT(18, 0x1FFFFF);
				UTF8_ADD(12, 0x3FFFF);
				UTF8_MOVE();
				UTF8_ADD(6, 0xFFF);
				UTF8_MOVE();
				UTF8_ADD_MASK(0x3F);
				++it;
				return true;
			}

			it = save;
			return false;
		}
	}

#undef UTF8_TAKE_FIRST
#undef UTF8_MOVE
#undef UTF8_SHIFT
#undef UTF8_ADD
#undef UTF8_ADD_MASK

	template <typename octet_iterator, typename u32bit_iterator>
	u32bit_iterator to32(octet_iterator start, octet_iterator end, u32bit_iterator result)
	{
		while (start != end)
		{
			uint32_t code_point;
			if (!impl::next(start, end, code_point))
			{
				code_point = UCS_REPLACEMENT;
				++start;
			}
			*result++ = code_point;
		}

		return result;
	}

	inline ustring to32(const string& s)
	{
		ustring out;
		out.reserve(s.length());
		to32(s.begin(), s.end(), std::back_inserter(out));
		return out;
	}
}

#endif // __GFX_UTF8_HPP__
