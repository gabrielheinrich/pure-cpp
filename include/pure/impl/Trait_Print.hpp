#pragma once

#include <pure/traits.hpp>
#include <pure/support/identifier.hpp>
#include <pure/support/tuple.hpp>

namespace pure {
	namespace detail {

		template<typename Stream, typename Child>
		void print_child (Stream& stream, const Child& child) {
			if constexpr (Trait_CString<Child>::implemented) {
				if (pure::category_id (child) == String.id) {
					IO::printf_to (stream, "\"%s\"", raw_cstring (child));
					return;
				}
			}
			assert (pure::category_id (child) != String.id);

			if (pure::category_id (child) == Character.id) {
				IO::write_raw_byte (stream, '\'');
				IO::print_to (stream, child);
				IO::write_raw_byte (stream, '\'');
			}
			else {
				IO::print_to (stream, child);
			}
		}

		template<typename Stream, typename T>
		void print_sequence_to (Stream& stream, T&& enumerator) {
			if (enumerator.empty ()) {
				IO::write_raw_string (stream, "[]");
				return;
			}
			IO::write_raw_string (stream, "[");
			print_child (stream, enumerator.read ());
			enumerator.next ();
			for (; !enumerator.empty (); enumerator.next ()) {
				IO::write_raw_string (stream, ", ");
				print_child (stream, enumerator.read ());
			}
			IO::write_raw_string (stream, "]");
		};

		namespace tuple {
			template<intptr_t index, intptr_t count, typename Stream, typename T>
			void print_to_loop (Stream& stream, const T& self) {
				if constexpr (index == count) return;
				else {
					IO::write_raw_string (stream, ", ");
					print_child (stream, tuple::nth<index> (self));
					return print_to_loop<index + 1, count> (stream, self);
				}
			};

			template<typename Stream, typename T>
			void print_to (Stream& stream, const T& self) {
				constexpr auto count = tuple::count<T>;
				if constexpr (count == 0) {
					IO::write_raw_string (stream, "[]");
					return;
				}
				else {
					IO::write_raw_string (stream, "[");
					print_child (stream, tuple::nth<0> (self));
					print_to_loop<1, count> (stream, self);
					IO::write_raw_string (stream, "]");
				}
			};
		}
		template<intptr_t index, typename Stream, typename T>
		void record_print_to_loop (Stream& stream, const T& self) {
			if constexpr (index == T::count) return;
			else {
				IO::write_raw_string (stream, ", ");
				print_child (stream, self.template nth_id<index> ());
				IO::write_raw_string (stream, " : ");
				print_child (stream, self.template nth_element<index> ());
			}
		};

		template<typename Stream, typename T>
		void record_print_to (Stream& stream, const T& self) {
			if constexpr (T::count == 0) {
				IO::write_raw_string (stream, "{}");
				return;
			}
			else {
				IO::write_raw_string (stream, "{");
				print_child (stream, self.template nth_id<0> ());
				IO::write_raw_string (stream, " : ");
				print_child (stream, self.template nth_element<0> ());
				record_print_to_loop<1> (stream, self);
				IO::write_raw_string (stream, "}");
			}
		};

		template<typename Stream, typename T>
		void print_map_to (Stream& stream, T&& self) {
			if (self.empty ()) {
				IO::write_raw_string (stream, "{}");
				return;
			}
			else {
				IO::write_raw_string (stream, "{");
				auto&& element = self.read ();
				print_child (stream, pure::first (element));
				IO::write_raw_string (stream, " : ");
				print_child (stream, pure::second (element));
				self.next ();
				for (; !self.empty (); self.next ()) {
					auto&& element = self.read ();
					IO::write_raw_string (stream, ", ");
					print_child (stream, pure::first (element));
					IO::write_raw_string (stream, " : ");
					print_child (stream, pure::second (element));
				}
				IO::write_raw_string (stream, "}");
			}
		};
	}
}

namespace pure {
	template<typename T>
	struct Trait_Print<T, Type_Class::Nil> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			IO::write_raw_string (stream, "nullptr");
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::Bool> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			IO::write_raw_string (stream, self ? "true" : "false");
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::Int> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			if constexpr (std::is_signed_v<T> && sizeof (T) <= sizeof (intptr_t))
				IO::printf_to (stream, "%ti", intptr_t (self));
			else
				IO::printf_to (stream, "%lli", int64_t (self));
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::Double> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			char buf[32];
			auto len = sprintf (buf, "%.16g", double (self));
			if (strspn (buf, "0123456789-") == len) {
				buf[len] = '.';
				buf[len + 1] = '0';
				buf[len + 2] = '\0';
			}
			IO::write_raw_string (stream, buf);
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::Character> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			if (uint32_t (self) < 0x80) {
				IO::printf_to (stream, "%c", char (self));
			}
			else {
				char result[8];
				char* end = utf8::write_char (result, char32_t (self));
				end[0] = '\0';
				IO::printf_to (stream, "%s", result);
			};
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::CString> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			IO::write_raw_string (stream, self);
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::Identifier> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			IO::write_raw_string (stream, self.string);
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::Var> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			switch (self.tag ()) {
				case Var::Tag::Nil : return IO::print_to (stream, nullptr);
				case Var::Tag::False : return IO::print_to (stream, false);
				case Var::Tag::True : return IO::print_to (stream, true);
				case Var::Tag::Int : return IO::print_to (stream, self.get_int ());
				case Var::Tag::Int64 : return IO::print_to (stream, self.get_int64 ());
				case Var::Tag::Double : return IO::print_to (stream, self.get_double ());
				case Var::Tag::Char : return IO::print_to (stream, self.get_char ());
				case Var::Tag::String : return IO::print_to (stream, self.get_cstring ());
				case Var_Tag_Pointer : return self->print_to (stream);
				default : throw operation_not_supported ();
			}
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::Exception> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			IO::printf_to (stream, "ERROR (%s)", self.what ());
		}
	};

	template<typename T, typename... Elements>
	struct Trait_Print<T, Type_Class::Tuple<Elements...>> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			detail::tuple::print_to (stream, self);
		}
	};

	template<typename T, typename Capabilities>
	struct Trait_Print<T, Type_Class::Iterable<Capabilities>> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			detail::print_sequence_to (stream, pure::enumerate (self));
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::Sequence> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			detail::print_sequence_to (stream, pure::enumerate (self));
		}
	};

	template<typename T>
	struct Trait_Print<T, Type_Class::Record> : Trait_Definition {
		template<typename Stream>
		static void print_to (Stream& stream, const T& self) {
			detail::record_print_to (stream, self);
		}
	};
}
