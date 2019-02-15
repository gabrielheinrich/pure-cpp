#pragma once

#include <pure/traits.hpp>

namespace pure {
	template<typename T>
	struct Trait_Stream<T, Type_Class::File_Stream> : Trait_Definition {
		static bool write_raw_bytes (T& self, const uint8_t* data, intptr_t num_bytes) {
			return fwrite (data, 1, num_bytes, self) != EOF;
		}

		static bool write_raw_string (T& self, const char* data) {
			return fputs (data, self) != EOF;
		}

		static bool vprintf (T& self, const char* format, va_list& args) {
			return std::vfprintf (self, format, args) >= 0;
		}

		static bool write_raw_byte (T& self, uint8_t byte) {
			return fputc (byte, self) != EOF;
		}

		static bool stream_is_exhausted (T& self) {
			return feof (self);
		}
	};

	template<typename T>
	struct Trait_Stream<T, Type_Class::Stream> : Trait_Definition {
		static bool write_raw_bytes (T& self, const uint8_t* data, intptr_t num_bytes) {
			return self.write_raw_bytes (data, num_bytes);
		}

		static bool write_raw_string (T& self, const char* data) {
			return self.write_raw_string (data);
		}

		static bool vprintf (T& self, const char* format, va_list& args) {
			return self.vprintf (format, args);
		}

		static bool write_raw_byte (T& self, uint8_t byte) {
			return self.write_raw_byte (byte);
		}

		static bool stream_is_exhausted (T& self) {
			return self.is_exhausted ();
		}
	};

	template<typename T>
	struct Trait_Stream<T, Type_Class::Std_Stream> : Trait_Definition {
		static bool write_raw_bytes (T& self, const uint8_t* data, intptr_t num_bytes) {
			try {
				self.print_to (reinterpret_cast<const char*>(data), num_bytes);
				return self.good ();
			}
			catch (...) { return false; }
		}

		static bool write_raw_string (T& self, const char* data) {
			try {
				self << data;
				return self.good ();
			}
			catch (...) { return false; }
		}

		static bool vprintf (T& self, const char* format, va_list& args) {
			uint8_t buffer[1024];
			auto num_bytes = std::vsnprintf (reinterpret_cast<char*>(buffer), 1024, format, args);
			if (num_bytes < 0 || num_bytes >= 1023) return false;
			return write_raw_bytes (self, buffer, num_bytes);
		}

		static bool write_raw_byte (T& self, uint8_t byte) {
			try {
				self.put (byte);
				return self.good ();
			}
			catch (...) { return false; }
		}

		static bool stream_is_exhausted (T& self) {
			return self.eof ();
		}
	};

	template<typename T>
	struct Trait_Stream<T, Type_Class::Var> : Trait_Definition {
		static bool write_raw_bytes (T& self, const uint8_t* data, intptr_t num_bytes) {
			switch (self.tag ()) {
				case Var_Tag_Pointer : return self->write_raw_bytes (data, num_bytes);
				default : throw operation_not_supported ();
			}
		}

		static bool write_raw_string (T& self, const char* data) {
			switch (self.tag ()) {
				case Var_Tag_Pointer : return self->write_raw_string (data);
				default : throw operation_not_supported ();
			}
		}

		static bool vprintf (T& self, const char* format, va_list& args) {
			switch (self.tag ()) {
				case Var_Tag_Pointer : return self->vprintf (format, args);
				default : throw operation_not_supported ();
			}
		}

		static bool write_raw_byte (T& self, uint8_t byte) {
			switch (self.tag ()) {
				case Var_Tag_Pointer : return self->write_raw_byte (byte);
				default : throw operation_not_supported ();
			}
		}
	};

}

