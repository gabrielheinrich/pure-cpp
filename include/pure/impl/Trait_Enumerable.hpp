#pragma once

#include <pure/traits.hpp>
#include <pure/support/transience.hpp>
#include <pure/types/some.hpp>
#include <pure/object/basic_string.hpp>
#include <pure/support/record.hpp>
#include <pure/object/persistent_vector.hpp>
#include <pure/object/persistent_map.hpp>

namespace pure {
	template<typename T>
	struct Trait_Enumerable<T, Type_Class::CString> : Trait_Definition {
		static bool Enumerable (const T&) { return true; }

		static cstring_enumerator enumerate (const T& self) { return {self}; }

		static intptr_t count (const T& self) { return utf8::count (self); }
		static bool Empty (const T& self) { return *self == '\0'; }

		static char32_t nth (const T& self, intptr_t n) { return Basic::String::cstring_nth (self, n); }

		static char32_t first (const T& self) { return nth (self, 0); }
		static char32_t second (const T& self) { return nth (self, 1); }

		template<typename Value>
		static unique<Basic::String> append (const T& self, const Value& value) {
			if constexpr (std::is_convertible_v<Value, char32_t>) {
				intptr_t strlen = std::strlen (self);
				auto capacity = Basic::String::capacity_needed_for_length (strlen * 2);
				unique<Basic::String> result {
						new (Basic::String::capacity_to_num_bytes (capacity)) Basic::String {capacity, self, strlen}};
				result->append_char (value);
				return std::move (result);
			}
			else throw operation_not_supported ();
		};
	};

	template<typename T>
	struct Trait_Enumerable<T, Type_Class::Identifier> : Trait_Definition {
		static bool Enumerable (const T&) { return true; }

		static cstring_enumerator enumerate (const T& self) { return {self.string}; }

		static intptr_t count (const T& self) { return utf8::count (self.string); }
		static bool Empty (const T& self) { return self.length == 0; }

		static char32_t nth (const T& self, intptr_t n) { return Basic::String::cstring_nth (self.string, n); }

		static char32_t first (const T& self) { return nth (self, 0); }
		static char32_t second (const T& self) { return nth (self, 1); }

		template<typename Value>
		static unique<Basic::String> append (const T& self, const Value& value) {
			if constexpr (std::is_convertible_v<Value, char32_t>) {
				intptr_t strlen = self.length;
				auto capacity = Basic::String::capacity_needed_for_length (strlen * 2);
				unique<Basic::String> result {
						new (Basic::String::capacity_to_num_bytes (capacity)) Basic::String {capacity, self.string,
																							 strlen}};
				result->append_char (value);
				return std::move (result);
			}
			else throw operation_not_supported ();
		};
	};

	template<typename T>
	struct Trait_Enumerable<T, Type_Class::Var> : Trait_Definition {
		static bool Enumerable (const T& self) {
			switch (self.tag ()) {
				case Var::Tag::String : return true;
				case Var_Tag_Pointer : return self->Enumerable ();
				default : return false;
			}
		}

		static auto enumerate (const T& self) {
			if constexpr (T::definitely_pointer) {
				return self->enumerate ();
			}
			else {
				using return_type = std::conditional_t<std::is_same_v<decltype (self->enumerate ()), cstring_enumerator>, cstring_enumerator, generic_enumerator>;
				switch (self.tag ()) {
					case Var::Tag::String : return return_type (pure::enumerate (self.get_cstring ()));
					case Var_Tag_Pointer : return return_type (self->enumerate ());
					default : throw operation_not_supported ();
				}
			}
		}

		static intptr_t count (const T& self) {
			switch (self.tag ()) {
				case Var::Tag::String : return pure::count (self.get_cstring ());
				case Var_Tag_Pointer : return self->count ();
				default : throw operation_not_supported ();
			}
		}

		static bool Empty (const T& self) {
			switch (self.tag ()) {
				case Var::Tag::String : return pure::Empty (self.get_cstring ());
				case Var_Tag_Pointer : return self->Empty ();
				default : throw operation_not_supported ();
			}
		}

		static auto nth (const T& self, intptr_t n) {
			if constexpr (T::definitely_pointer) {
				return self->nth (n);
			}
			else {
				using return_type = unify_types<char32_t, decltype (self->nth (n))>;
				switch (self.tag ()) {
					case Var::Tag::String : return return_type (pure::nth (self.get_cstring (), n));
					case Var_Tag_Pointer : return return_type (self->nth (n));
					default : throw operation_not_supported ();
				}
			}
		}

		static auto first (const T& self) {
			if constexpr (T::definitely_pointer) {
				return self->first ();
			}
			else {
				using return_type = unify_types<char32_t, decltype (self->first ())>;
				switch (self.tag ()) {
					case Var::Tag::String : return return_type (pure::first (self.get_cstring ()));
					case Var_Tag_Pointer : return return_type (self->first ());
					default : throw operation_not_supported ();
				}
			}
		}

		static auto second (const T& self) {
			if constexpr (T::definitely_pointer) {
				return self->second ();
			}
			else {
				using return_type = unify_types<char32_t, decltype (self->second ())>;
				switch (self.tag ()) {
					case Var::Tag::String : return return_type (pure::second (self.get_cstring ()));
					case Var_Tag_Pointer : return return_type (self->second ());
					default : throw operation_not_supported ();
				}
			}
		}

		using object_type = typename T::object_type;

		template<typename... Args>
		using append_persistent_t = decltype (std::declval<object_type> ().append_persistent (std::declval<T> (),
																							  std::declval<Args> ()...));
		template<typename... Args>
		using append_transient_t = decltype (std::declval<object_type> ().append_transient (std::declval<T> (),
																							std::declval<Args> ()...));

		template<typename TT, typename... Args>
		static auto append (TT&& self, Args&& ... args)
		-> detail::transience_return_type<TT&&, append_transient_t<Args && ...>, append_persistent_t<
				Args && ...>, some<>> {
			using namespace detail;
			using transient_t = append_transient_t<Args&& ...>;
			using persistent_t = append_persistent_t<Args&& ...>;

			using path = transience_path<TT&&, transient_t, persistent_t>;

			if constexpr (std::is_same_v<path, transience::persistent>) {
				switch (self.tag ()) {
					case Var_Tag_Pointer : return self->append_persistent (self, std::forward<Args> (args)...);
					default : throw operation_not_supported ();
				}
			}
			else if constexpr (std::is_same_v<path, transience::transient>) {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						return transient_return (
								self->append_transient (std::move (self), std::forward<Args> (args)...), self);
					default : throw operation_not_supported ();
				}
			}
			else if constexpr (std::is_same_v<path, transience::resettable_transient>) {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						self.transient_reset (self->append_transient (std::move (self), std::forward<Args> (args)...));
					default : throw operation_not_supported ();
				}
			}
			else if constexpr (std::is_same_v<path, transience::some_pointer>) {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						if constexpr (detail::is_moveable<TT&&>)
							return transient_return (
									self->append_transient (std::move (self), std::forward<Args> (args)...));
					case Var::Tag::Shared :
					case Var::Tag::Interned :
					case Var::Tag::Weak :
						return self->append_persistent (std::move (self), std::forward<Args> (args)...);
					default : throw operation_not_supported ();
				}
			}
			else if constexpr (std::is_same_v<path, transience::resettable_pointer>) {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						return self->transient_reset (
								self->append_transient (std::move (self), std::forward<Args> (args)...));
					case Var::Tag::Shared :
					case Var::Tag::Interned :
					case Var::Tag::Weak :
						return self->transient_reset (
								self->append_persistent (std::move (self), std::forward<Args> (args)...));
					default : throw operation_not_supported ();
				}
			}
			else {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						if constexpr (detail::is_moveable<T&&>)
							return transient_return (
									self->append_transient (std::move (self), std::forward<Args> (args)...), self);
					case Var::Tag::Shared :
					case Var::Tag::Interned :
					case Var::Tag::Weak :
						return self->append_persistent (std::move (self), std::forward<Args> (args)...);
					case Var::Tag::String : return pure::append (self.get_cstring (), std::forward<Args> (args)...);
					default : throw operation_not_supported ();
				}
			}
		};
	};

	template<typename T, typename... Elements>
	struct Trait_Enumerable<T, Type_Class::Tuple<Elements...>> : Trait_Definition {
		static bool Enumerable (const T&) { return true; }

		static auto enumerate (const T& self) { return detail::tuple::enumerator<T, Elements...> {self}; }

		static intptr_t count (const T& self) { return sizeof... (Elements); }
		static bool Empty (const T& self) { return sizeof... (Elements) == 0; }

		static auto nth (const T& self, intptr_t n) { return detail::tuple::generic_nth (self, n); }

		static auto first (const T& self) {
			if constexpr (sizeof... (Elements) == 0) {
				throw operation_not_supported ();
				return nullptr;
			}
			else return detail::tuple::nth<0> (self);
		}
		static auto second (const T& self) {
			if constexpr (sizeof... (Elements) <= 1) {
				throw operation_not_supported ();
				return nullptr;
			}
			else return detail::tuple::nth<1> (self);
		}

		template<typename TT, typename Value>
		static auto append (TT&& self, Value&& value) {
			Persistent::Vector<var> result {std::forward<TT> (self)};
			return std::move (result).append_transient ({}, std::forward<Value> (value));
		};
	};

	template<typename T, typename Capabilities>
	struct Trait_Enumerable<T, Type_Class::Iterable<Capabilities>> : Trait_Definition {
		static bool Enumerable (const T&) { return true; }
		static auto enumerate (const T& self) {
			return iterator_range<decltype (self.begin ()), decltype (self.end ())> {self.begin (), self.end ()};
		}

		static intptr_t count (const T& self) {
			if constexpr (Capabilities::sized) return self.size ();
			else {
				intptr_t result = 0;
				auto end = self.end ();
				for (auto iter = self.begin (); iter != end; ++iter) {
					++result;
				}
				return result;
			}
		}
		static intptr_t Empty (const T& self) { return self.begin () == self.end (); }
		static auto nth (const T& self, intptr_t n) {
			if constexpr (Capabilities::random_access) {
				assert (self.begin () + n < self.end ());
				return *(self.begin () + n);
			}
			else {
				auto iter = self.begin ();
				while (n > 0) {
					assert (iter != self.end ());
					++iter;
					--n;
				}
				assert (iter != self.end ());
				return *iter;
			}
		}
		static auto first (const T& self) { return nth (self, 0); }
		static auto second (const T& self) { return nth (self, 1); }

		template<typename TT, typename Value>
		static auto append (TT&& self, Value&& value) {
			Persistent::Vector<var> result {std::forward<TT> (self)};
			return std::move (result).append_transient ({}, std::forward<Value> (value));
		}
	};

	template<typename T>
	struct Trait_Enumerable<T, Type_Class::Sequence> : Trait_Definition {
		static bool Enumerable (const T&) { return true; }
		static auto enumerate (const T& self) {
			return self.enumerate ();
		}

		static intptr_t count (const T& self) {
			auto enumerator = self.enumerate ();
			if constexpr (enumerator.has_size ()) return enumerator.size ();
			else {
				intptr_t result = 0;
				for (; !enumerator.empty (); enumerator.next ()) {
					++result;
				}
				return result;
			}
		}
		static intptr_t Empty (const T& self) { return self.enumerate ().empty (); }
		static auto nth (const T& self, intptr_t n) {
			auto enumerator = self.enumerate ();
			for (; n > 0; enumerator.next ()) {
				if (enumerator.empty ()) throw operation_not_supported ();
				--n;
			}
			if (enumerator.empty ()) throw operation_not_supported ();
			return enumerator.read ();
		}

		static auto first (const T& self) { return nth (self, 0); }
		static auto second (const T& self) { return nth (self, 1); }

		template<typename TT, typename Value>
		static auto append (TT&& self, Value&& value) {
			Persistent::Vector<var> result {std::forward<TT> (self)};
			return std::move (result).append_transient ({}, std::forward<Value> (value));
		}
	};


	template<typename T>
	struct Trait_Enumerable<T, Type_Class::Record> : Trait_Definition {
		static bool Enumerable (const T&) { return true; }

		static auto enumerate (const T& self) { return self.enumerate (); }

		static intptr_t count (const T&) { return T::count; }
		static intptr_t Empty (const T&) { return T::count == 0; }
		static auto nth (const T& self, intptr_t n) { return self.nth_pair (n); }

		static auto first (const T& self) { return self.template nth_pair<0> (); }
		static auto second (const T& self) { return self.template nth_pair<1> (); }

		template<typename TT, typename Value>
		static auto append (TT&& self, Value&& value) {
			throw operation_not_supported ();
			return self;
		};
	};
}
