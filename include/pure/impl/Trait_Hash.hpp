#pragma once

#include <pure/support/hashing.hpp>
#include <pure/support/identifier.hpp>
#include <pure/traits.hpp>
#include <pure/support/tuple.hpp>
#include <pure/support/record.hpp>

namespace pure {
	namespace detail {
		template<typename T>
		int32_t hash_sequence (T&& enumerator) {
			int32_t result = 0x1;
			for (; !enumerator.empty (); enumerator.next ()) {
				result = murmur3::hash_combine_ordered (result, pure::hash (enumerator.read ()));
			}
			return murmur3::fmix (result);
		}

		namespace tuple {
			template<intptr_t index, intptr_t count, typename T>
			int32_t hash (const T& self, int32_t result = 0x1) {
				if constexpr (index == count) return murmur3::fmix (result);
				else return hash<index + 1, count> (self, murmur3::hash_combine_ordered (result, pure::hash (
							tuple::nth<index> (self))));
			}
		}

		template<typename First, typename Second>
		int32_t hash_pair (const First& first, const Second& second) {
			int32_t result = 0x1;
			result = murmur3::hash_combine_ordered (result, pure::hash (first));
			result = murmur3::hash_combine_ordered (result, pure::hash (second));
			return murmur3::fmix (result);
		};

		template<intptr_t index = 0, typename T>
		int32_t hash_record (const T& self, int32_t result = 0x3) {
			if constexpr (index == T::count) return murmur3::fmix (result);
			else return hash_record<index + 1> (self, murmur3::hash_combine_unordered (result, hash_pair (
						self.template nth_id<index> (), self.template nth_element<index> ())));
		};

		template<typename T>
		int32_t hash_enumerable_unordered (const T& self) {
			int32_t result = 0x3;
			for (auto enumerator = pure::enumerate (self); !enumerator.empty (); enumerator.next ()) {
				result = murmur3::hash_combine_unordered (result, pure::hash (enumerator.read ()));
			}
			return murmur3::fmix (result);
		}

		template<typename T>
		int32_t hash_map (T&& enumerator) {
			int32_t result = 0x3;
			for (; !enumerator.empty (); enumerator.next ()) {
				auto&& element = enumerator.read ();
				result = murmur3::hash_combine_unordered (result,
														  hash_pair (pure::first (element), pure::second (element)));
			}
			return murmur3::fmix (result);
		}
	}
}

namespace pure {
	template<typename T>
	struct Trait_Hash<T, Type_Class::Nil> : Trait_Definition {
		static int32_t hash (const T& self) { return 0; }
	};

	template<typename T>
	struct Trait_Hash<T, Type_Class::Bool> : Trait_Definition {
		static int32_t hash (const T& self) { return self ? 1231 : 1237; }
	};

	template<typename T>
	struct Trait_Hash<T, Type_Class::Int> : Trait_Definition {
		static int32_t hash (const T& self) {
			if constexpr (sizeof (T) < sizeof (int32_t) || (sizeof (T) == sizeof (int32_t) && std::is_signed_v<T>))
				return detail::murmur3::hash_int32 (self, 0);
			else
				return detail::murmur3::hash_int64 (self, 0);
		}
	};

	template<typename T>
	struct Trait_Hash<T, Type_Class::Double> : Trait_Definition {
		static int32_t hash (const T& self) {
			if ((double)(int64_t)self == self) return detail::murmur3::hash_int64 ((int64_t)self, 0);
			else return 0;
		}
	};

	template<typename T>
	struct Trait_Hash<T, Type_Class::Character> : Trait_Definition {
		static int32_t hash (const T& self) {
			return detail::murmur3::hash_int32 (self, 0x3A6F10C7);
		}
	};

	namespace detail {
		int32_t hash_cstring_with_length (const char* cstr, intptr_t length) {
			return detail::murmur3::hash_raw_bytes (cstr, length, 0xCE7E9683);
		}
	}
	template<typename T>
	struct Trait_Hash<T, Type_Class::CString> : Trait_Definition {
		static int32_t hash (const T& self) {
			return detail::hash_cstring_with_length (self, std::strlen (self));
		}
	};

	template<typename T>
	struct Trait_Hash<T, Type_Class::Identifier> : Trait_Definition {
		static int32_t hash (const T& self) {
			return detail::hash_cstring_with_length (self.string, self.length);
		}
	};

	template<typename T>
	struct Trait_Hash<T, Type_Class::Var> : Trait_Definition {
		static int32_t hash (const T& self) {
			switch (self.tag ()) {
				case Var::Tag::Nil : return pure::hash (nullptr);
				case Var::Tag::False : return pure::hash (false);
				case Var::Tag::True : return pure::hash (true);
				case Var::Tag::Int : return pure::hash (self.get_int ());
				case Var::Tag::Int64 : return pure::hash (self.get_int64 ());
				case Var::Tag::Double : return pure::hash (self.get_double ());
				case Var::Tag::Char : return pure::hash (self.get_char ());
				case Var::Tag::String : return pure::hash (self.get_cstring ());
				case Var_Tag_Pointer : return self->hash ();
				default : return 0;
			}
		}
	};

	template<typename T, typename... Elements>
	struct Trait_Hash<T, Type_Class::Tuple<Elements...>> : Trait_Definition {
		static int32_t hash (const T& self) {
			return detail::tuple::hash<0, sizeof... (Elements)> (self);
		}
	};

	template<typename T, typename Capabilities>
	struct Trait_Hash<T, Type_Class::Iterable<Capabilities>> : Trait_Definition {
		static int32_t hash (const T& self) {
			return detail::hash_sequence (pure::enumerate (self));
		}
	};

	template<typename T>
	struct Trait_Hash<T, Type_Class::Sequence> : Trait_Definition {
		static int32_t hash (const T& self) {
			return detail::hash_sequence (pure::enumerate (self));
		}
	};

	template<typename T>
	struct Trait_Hash<T, Type_Class::Record> : Trait_Definition {
		static int32_t hash (const T& self) {
			return detail::hash_record (self);
		}
	};
}
