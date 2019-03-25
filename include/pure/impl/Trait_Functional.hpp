#pragma once

#include <pure/traits.hpp>
#include <pure/support/transience.hpp>
#include <pure/types/some.hpp>
#include <pure/object/basic_string.hpp>
#include <pure/support/tuple.hpp>
#include <pure/support/record.hpp>
#include <pure/object/persistent_vector.hpp>
#include <pure/object/persistent_map.hpp>

namespace pure {
	namespace detail {
		template<typename... Args>
		struct Sequence_Applicable : std::false_type {};

		template<typename Index>
		struct Sequence_Applicable<Index> {
			static constexpr bool value = std::is_convertible_v<Index, intptr_t>;
		};

	}
	template<typename T>
	struct Trait_Functional<T, Type_Class::CString> : Trait_Definition {
		template<typename Index>
		static char32_t apply (const T& self, const Index& index) {
			if constexpr (std::is_convertible_v<Index, intptr_t>) {
				return Basic::String::cstring_nth (self, index);
			}
			else
				throw operation_not_supported ();
		}

		template<typename... Args>
		static constexpr bool Applicable = detail::Sequence_Applicable<Args...>::value;

		static intptr_t arity (const T& self) { return 1; }
		static bool Variadic (const T& self) { return false; }

		template<typename Index, typename Value>
		static unique<Basic::String> set (const T& self, const Index& index, const Value& value) {
			if constexpr (std::is_convertible_v<Index, intptr_t> && std::is_convertible_v<Value, char32_t>) {
				intptr_t strlen = std::strlen (self);
				auto capacity = Basic::String::capacity_needed_for_length (strlen + 4);
				unique<Basic::String> result {
						new (Basic::String::capacity_to_num_bytes (capacity)) Basic::String {capacity, self, strlen}};
				//TODO append head, char, tail to empty
				auto _ = result->set_transient (std::move (result), index, value);
				assert (Nil (_));
				return std::move (result);

			}
			else throw operation_not_supported ();
		};
	};

	template<typename T>
	struct Trait_Functional<T, Type_Class::Identifier> : Trait_Definition {
		template<typename Index>
		static char32_t apply (const T& self, const Index& index) {
			if constexpr (std::is_convertible_v<Index, intptr_t>) {
				return Basic::String::cstring_nth (self.string, index);
			}
			else
				throw operation_not_supported ();
		}

		template<typename... Args>
		static constexpr bool Applicable = detail::Sequence_Applicable<Args...>::value;

		static intptr_t arity (const T& self) { return 1; }
		static bool Variadic (const T& self) { return false; }

		template<typename Index, typename Value>
		static unique<Basic::String> set (const T& self, const Index& index, const Value& value) {
			return pure::set (self.string, index, value);
		};
	};

	template<typename T>
	struct Trait_Functional<T, Type_Class::Var> : Trait_Definition {
		template<typename... Args>
		static constexpr bool Applicable = true;

		template<typename... Args>
		static auto apply (const T& self, Args&& ... args) {
			if constexpr (T::definitely_pointer) {
				return self->apply (std::forward<Args> (args)...);
			}
			else {
				switch (self.tag ()) {
					case Var::Tag::String :
						return var (pure::apply (self.get_cstring (), std::forward<Args> (args)...));
					case Var_Tag_Pointer : return var (self->apply (std::forward<Args> (args)...));
					default : throw operation_not_supported ();
				}
			}
		}

		static intptr_t arity (const T& self) {
			switch (self.tag ()) {
				case Var::Tag::String : return 1;
				case Var_Tag_Pointer : return self->arity ();
				default : throw operation_not_supported ();
			}
		}

		static bool Variadic (const T& self) {
			switch (self.tag ()) {
				case Var::Tag::String : return false;
				case Var_Tag_Pointer : return self->Variadic ();
				default : throw operation_not_supported ();
			}
		}

		using object_type = typename T::object_type;

		template<typename... Args>
		using set_persistent_t = decltype (std::declval<object_type> ().set_persistent (std::declval<T> (),
																						std::declval<Args> ()...));
		template<typename... Args>
		using set_transient_t = decltype (std::declval<object_type> ().set_transient (std::declval<T> (),
																					  std::declval<Args> ()...));

		template<typename TT, typename... Args>
		static auto set (TT&& self, Args&& ... args)
		-> detail::transience_return_type<TT&&, set_transient_t<Args && ...>, set_persistent_t<Args && ...>, some<>> {
			using namespace detail;
			using transient_t = set_transient_t<Args&& ...>;
			using persistent_t = set_persistent_t<Args&& ...>;

			using path = transience_path<TT&&, transient_t, persistent_t>;

			if constexpr (std::is_same_v<path, transience::persistent>) {
				switch (self.tag ()) {
					case Var_Tag_Pointer : return self->set_persistent (self, std::forward<Args> (args)...);
					default : throw operation_not_supported ();
				}
			}
			else if constexpr (std::is_same_v<path, transience::transient>) {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						return transient_return (self->set_transient (std::move (self), std::forward<Args> (args)...),
												 self);
					default : throw operation_not_supported ();
				}
			}
			else if constexpr (std::is_same_v<path, transience::resettable_transient>) {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						self.transient_reset (self->set_transient (std::move (self), std::forward<Args> (args)...));
					default : throw operation_not_supported ();
				}
			}
			else if constexpr (std::is_same_v<path, transience::some_pointer>) {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						if constexpr (detail::is_moveable<TT&&>)
							return transient_return (
									self->set_transient (std::move (self), std::forward<Args> (args)...));
					case Var::Tag::Shared :
					case Var::Tag::Interned :
					case Var::Tag::Weak : return self->set_persistent (std::move (self), std::forward<Args> (args)...);
					default : throw operation_not_supported ();
				}
			}
			else if constexpr (std::is_same_v<path, transience::resettable_pointer>) {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						return self->transient_reset (
								self->set_transient (std::move (self), std::forward<Args> (args)...));
					case Var::Tag::Shared :
					case Var::Tag::Interned :
					case Var::Tag::Weak :
						return self->transient_reset (
								self->set_persistent (std::move (self), std::forward<Args> (args)...));
					default : throw operation_not_supported ();
				}
			}
			else {
				switch (self.tag ()) {
					case Var::Tag::Unique :
					case Var::Tag::Moveable :
						if constexpr (detail::is_moveable<T&&>)
							return transient_return (
									self->set_transient (std::move (self), std::forward<Args> (args)...), self);
					case Var::Tag::Shared :
					case Var::Tag::Interned :
					case Var::Tag::Weak : return self->set_persistent (std::move (self), std::forward<Args> (args)...);
					case Var::Tag::String : return pure::set (self.get_cstring (), std::forward<Args> (args)...);
					default : throw operation_not_supported ();
				}
			}
		};
	};

	template<typename T, typename R, typename... FArgs>
	struct Trait_Functional<T, Type_Class::Function<R, FArgs...>> : Trait_Definition {
		template<typename... Args>
		static constexpr bool Applicable = !std::is_same_v<std::false_type, decltype (detail::check_call (
				std::declval<T> (), std::declval<Args> ()...))>;

		template<typename... Args>
		static auto apply (const T& self, Args&& ... args) {
			if constexpr (sizeof... (Args) == sizeof... (FArgs) && Applicable<Args...>)
				return self (std::forward<Args> (args)...);
			else
				throw operation_not_supported();
		}

		static intptr_t arity (const T&) { return sizeof... (FArgs); }

		static bool Variadic (const T&) { return false; }

		template<typename... Args>
		static var set (const T&, Args&& ...) { throw operation_not_supported (); }
	};

	template<typename T, typename R, intptr_t Arity, bool is_Variadic>
	struct Trait_Functional<T, Type_Class::Generic_Function<R, Arity, is_Variadic>> : Trait_Definition {
		template<typename... Args>
		static constexpr bool Applicable = !std::is_same_v<std::false_type, decltype (detail::check_call (
				std::declval<T> (), std::declval<Args> ()...))>;

		template<typename... Args>
		static auto apply (const T& self, Args&& ... args) {
			if constexpr (sizeof... (Args) == Arity && Applicable<Args...>)
				return self (std::forward<Args> (args)...);
			else
				throw operation_not_supported();
		}

		static intptr_t arity (const T&) { return Arity; }

		static bool Variadic (const T&) { return is_Variadic; }

		template<typename... Args>
		static var set (const T&, Args&& ...) { throw operation_not_supported (); }
	};

	template<typename T>
	struct Trait_Functional<T, Type_Class::Symbolic_Set> : Trait_Definition {
		template<typename... Args>
		static constexpr bool Applicable = sizeof... (Args) == 1;

		template<typename... Args>
		static auto apply (const T& self, Args&& ... args) {
			if constexpr (sizeof... (Args) == 1)
				return self (args...);
			else
				throw operation_not_supported();
		}

		static intptr_t arity (const T&) { return 1; }
		static bool Variadic (const T&) { return false; }

		template<typename... Args>
		static var set (const T&, Args&& ...) { throw operation_not_supported (); }
	};

	template<typename T, typename... Elements>
	struct Trait_Functional<T, Type_Class::Tuple<Elements...>> : Trait_Definition {
		template<typename... Args>
		static constexpr bool Applicable = detail::Sequence_Applicable<Args...>::value;

		template<typename... Args>
		static auto apply (const T& self, Args&& ... args) {
			if constexpr (sizeof... (Args) == 1)
				return detail::tuple::generic_nth (self, std::forward<Args> (args)...);
			else
				throw operation_not_supported();
		}

		static intptr_t arity (const T& self) { return 1; }
		static bool Variadic (const T& self) { return false; }

		template<typename TT, typename Index, typename Value>
		static auto set (TT&& self, const Index& index, Value&& value) {
			Persistent::Vector<var> result {std::forward<TT> (self)};
			return result.set_transient ({}, index, std::forward<Value> (value));
		}
	};

	template<typename T, typename Capabilities>
	struct Trait_Functional<T, Type_Class::Iterable<Capabilities>> : Trait_Definition {
		template<typename... Args>
		static constexpr bool Applicable = detail::Sequence_Applicable<Args...>::value;

		template<typename... Args>
		static auto apply (const T& self, Args&& ... args) {
			if constexpr (sizeof... (Args) == 1)
				return pure::nth (self, std::forward<Args> (args)...);
			else
				throw operation_not_supported();
		}

		static intptr_t arity (const T& self) { return 1; }
		static bool Variadic (const T& self) { return false; }

		template<typename TT, typename Index, typename Value>
		static auto set (TT&& self, const Index& index, Value&& value) {
			Persistent::Vector<var> result {std::forward<TT> (self)};
			return result.set_transient ({}, index, std::forward<Value> (value));
		}
	};

	template<typename T>
	struct Trait_Functional<T, Type_Class::Sequence> : Trait_Definition {
		template<typename... Args>
		static constexpr bool Applicable = detail::Sequence_Applicable<Args...>::value;

		template<typename... Args>
		static auto apply (const T& self, Args&& ... args) {
			if constexpr (sizeof... (Args) == 1)
				return pure::nth (self, std::forward<Args> (args)...);
			else
				throw operation_not_supported();
		}

		static intptr_t arity (const T& self) { return 1; }
		static bool Variadic (const T& self) { return false; }

		template<typename TT, typename Index, typename Value>
		static auto set (TT&& self, const Index& index, Value&& value) {
			Persistent::Vector<var> result {std::forward<TT> (self)};
			return result.set_transient ({}, index, std::forward<Value> (value));
		}
	};

	template<typename T>
	struct Trait_Functional<T, Type_Class::Record> : Trait_Definition {
		template<typename... Args>
		static constexpr bool Applicable = sizeof... (Args) == 1;

		template<typename... Args>
		static auto apply (const T& self, Args&&... args) {
			if constexpr (sizeof... (Args) == 1)
				return self (args...);
			else
				throw operation_not_supported();
		}

		static intptr_t arity (const T& self) { return 1; }
		static bool Variadic (const T& self) { return false; }

		template<typename TT, typename Index, typename Value>
		static auto set (TT&& self, Index&& index, Value&& value) {
			Persistent::Map<var, var> result {std::forward<TT> (self)};
			return result.set_transient ({}, std::forward<Index> (index), std::forward<Value> (value));
		}
	};
}
