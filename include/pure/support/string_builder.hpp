#pragma once

#include <pure/object/basic_string.hpp>
#include <pure/types/unique.hpp>

namespace pure::detail {

	struct string_builder : implements<Type_Class::Stream> {
		unique<Basic::String> result;

		string_builder (intptr_t capacity = 256) : result {Basic::String::create_empty (capacity)} {}
		//string_builder (const string_builder& other) : result {other.result} {};

		void reserve (intptr_t num_bytes) {
			if (result->remaining_capacity () >= num_bytes) return;
			auto new_capacity = result->capacity () * 2;
			while (new_capacity - result->cstring_length () < num_bytes)
				new_capacity *= 2;
			result = Basic::String::create_from_cstring (new_capacity, result->cstring (), result->cstring_length ());
		}

		bool write_raw_bytes (const uint8_t* data, intptr_t num_bytes) {
			reserve (num_bytes);
			result->append_cstring (reinterpret_cast<const char*> (data), num_bytes);
			return true;
		}

		bool write_raw_string (const char* data) {
			intptr_t length = std::strlen (data);
			reserve (length);
			result->append_cstring (reinterpret_cast<const char*> (data), length);
			return true;
		}

		bool write_raw_byte (uint8_t byte) {
			reserve (1);
			result->append_cstring (reinterpret_cast<const char*> (&byte), 1);
			return true;
		}

		bool vprintf (const char* format, va_list& args) {
			reserve (32);
			va_list args_copy;
			for (;;) {
				va_copy (args_copy, args);

				intptr_t cap = result->remaining_capacity ();
				intptr_t num_bytes = std::vsnprintf (result->str_end, cap + 1, format, args_copy);
				va_end (args_copy);

				if (num_bytes < 0) {
					return false;
				}
				else if (num_bytes > cap) {
					reserve (num_bytes + 32);
				}
				else {
					result->str_end += num_bytes;
					return true;
				}
			}
		}

		bool is_exhausted () const noexcept { return false; }

		unique<Basic::String> finish () { return result.release (); }
	};

}