#pragma once

#include <cstdint>
#include <cassert>

namespace pure::utf8 {
	static inline char32_t read_char (const char* str) {
		assert (str[0]);
		auto byte = static_cast<signed char>(str[0]);
		if (byte > 0) return str[0];

		auto n = static_cast<uint8_t> (byte);
		if ((n & (0x1 << 5)) == 0) {
			return (static_cast<uint32_t>((n & (0xff >> 3))) << 6) |
				   static_cast<uint32_t>(0x3f & static_cast<uint8_t>(str[1]));
		}
		if ((n & (0x1 << 4)) == 0) {
			return (static_cast<uint32_t>(n & (0xff >> 4)) << 12) |
				   (static_cast<uint32_t>(0x3f & static_cast<uint8_t>(str[1])) << 6) |
				   static_cast<uint32_t>(0x3f & static_cast<uint8_t>(str[2]));
		}
		return (static_cast<uint32_t>(n & (0xff >> 4)) << 18) |
			   (static_cast<uint32_t>(0x3f & static_cast<uint8_t>(str[1])) << 12) |
			   (static_cast<uint32_t>(0x3f & static_cast<uint8_t>(str[2])) << 6) |
			   static_cast<uint32_t>(0x3f & static_cast<uint8_t>(str[3]));
	}

	static inline char* advance (char* str) {
		assert (str[0]);
		auto byte = static_cast<signed char>(str[0]);
		if (byte > 0) return str + 1;

		auto n = static_cast<uint8_t> (byte);
		if ((n & (0x1 << 5)) == 0) return str + 2;
		if ((n & (0x1 << 4)) == 0) return str + 3;
		return str + 4;
	}

	static inline const char* advance (const char* str) {
		return advance (const_cast<char*>(str));
	}

	static inline const char* step_back (const char* ptr, const char* start) {
		do {
			if (ptr <= start) return nullptr;
			--ptr;
		} while ((*ptr & 0xC) == 0x80);
		return ptr;
	}

	static inline const char* advance_n (const char* str, intptr_t n) {
		while (n > 0) {
			if (*str == 0) return nullptr;
			str = advance (str);
			--n;
		}
		return str;
	}

	static inline char32_t read_char_and_advance (const char*& str) {
		auto byte = static_cast<signed char> (*str);

		if (byte >= 0) {
			str += 1;
			return static_cast<char32_t>(byte);
		}

		auto n = static_cast<uint32_t> (static_cast<uint8_t>(byte));
		auto data = str;
		if ((n & (0x1 << 5)) == 0) {
			str += 2;
			return (static_cast<uint32_t>((n & (0xff >> 3))) << 6) |
				   static_cast<uint32_t>(0x3f & static_cast<uint8_t>(data[1]));
		}
		if ((n & (0x1 << 4)) == 0) {
			str += 3;
			return (static_cast<uint32_t>(n & (0xff >> 4)) << 12) |
				   (static_cast<uint32_t>(0x3f & static_cast<uint8_t>(data[1])) << 6) |
				   static_cast<uint32_t>(0x3f & static_cast<uint8_t>(data[2]));
		}
		str += 4;
		return (static_cast<uint32_t>(n & (0xff >> 4)) << 18) |
			   (static_cast<uint32_t>(0x3f & static_cast<uint8_t>(data[1])) << 12) |
			   (static_cast<uint32_t>(0x3f & static_cast<uint8_t>(data[2])) << 6) |
			   static_cast<uint32_t>(0x3f & static_cast<uint8_t>(data[3]));

	}

	static inline intptr_t count (const char* str) {
		intptr_t count = 0;
		while (*str) {
			str = advance (str);
			++count;
		}

		return count;
	}

	static inline intptr_t bytes_required_for (char32_t character) {
		auto c = static_cast<uint32_t>(character);
		if (c < 0x80) return 1;
		if (c < 0x800) return 2;
		if (c < 0x10000) return 3;
		return 4;
	}

	static inline char* write_char (char* str, char32_t character) {
		auto c = static_cast<uint32_t>(character);

		if (c < 0x80) {
			str[0] = static_cast<char> (character);
			return str + 1;
		}
		else if (c < 0x800) {
			str[0] = static_cast<uint8_t> (static_cast<uint32_t>(0xff << 6) | (c >> 6));
			str[1] = static_cast<uint8_t> (static_cast<uint32_t>(0x80) | ((c >> 0) & (0xff >> 2)));
			return str + 2;
		}
		else if (c < 0x1000) {
			str[0] = static_cast<uint8_t> (static_cast<uint32_t>(0xff << 5) | (c >> 12));
			str[1] = static_cast<uint8_t> (static_cast<uint32_t>(0x80) | ((c >> 6) & (0xff >> 2)));
			str[2] = static_cast<uint8_t> (static_cast<uint32_t>(0x80) | ((c >> 0) & (0xff >> 2)));
			return str + 3;
		}
		else {
			str[0] = static_cast<uint8_t> (static_cast<uint32_t>(0xff << 5) | (c >> 18));
			str[1] = static_cast<uint8_t> (static_cast<uint32_t>(0x80) | ((c >> 12) & (0xff >> 2)));
			str[2] = static_cast<uint8_t> (static_cast<uint32_t>(0x80) | ((c >> 6) & (0xff >> 2)));
			str[3] = static_cast<uint8_t> (static_cast<uint32_t>(0x80) | ((c >> 0) & (0xff >> 2)));
			return str + 4;
		}
	}

	static int string_compare (const char* lhs, const char* rhs) {
		for (;;) {
			auto c1 = read_char_and_advance (lhs);
			auto c2 = read_char_and_advance (rhs);

			auto diff = static_cast<int32_t>(c1) - static_cast<int32_t>(c2);
			if (diff) return diff < 0 ? -1 : 1;
			else {
				if (c1 == 0)
					return c2 == 0 ? 0 : -1;
				else if (c2 == 0)
					return 1;
			}
		}
	}
}
