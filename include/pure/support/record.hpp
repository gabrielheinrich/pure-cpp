#pragma once

#include <pure/type_class.hpp>
#include <pure/support/type_utilities.hpp>
#include <pure/support/tuple.hpp>
#include <pure/support/identifier.hpp>
#include <pure/traits.hpp>
#include <pure/types/var.hpp>
#include <pure/types/interned.hpp>

namespace pure {
	namespace detail {
		template<typename Id_Elements_List, typename... Ts>
		struct record_tuple_t {
			static constexpr auto eval () {
				if constexpr (Id_Elements_List::empty) {
					return type_value<pure::tuple<Ts...>> {};
				}
				else {
					return record_tuple_t<typename Id_Elements_List::rest, Ts..., typename Id_Elements_List::first::second>::eval ();
				}
			}
			using value = typename decltype (record_tuple_t<Id_Elements_List, Ts...>::eval ())::value;
		};

		template<intptr_t n, typename Id_Elements_List>
		struct record_nth_id {
			static constexpr auto eval () {
				if constexpr (n == 0) {
					return type_value<typename Id_Elements_List::first::first> {};
				}
				else {
					return record_nth_id<n - 1, typename Id_Elements_List::rest>::eval ();
				}
			}
			using value = typename decltype (record_nth_id<n, Id_Elements_List>::eval ())::value;
		};

		template<typename Id, typename list, intptr_t index = 0>
		struct record_id_index {
			static constexpr intptr_t eval () {
				if constexpr (list::empty) return -1;
				else if constexpr (std::is_same_v<Id, typename list::first::first>) return index;
				else return record_id_index<Id, typename list::rest, index + 1>::eval ();
			}
		};

		template<intptr_t index = 0, typename Eq, typename T, typename U>
		bool record_equal_function (const Eq& eq, const T& self, const U& other) {
			if constexpr (index == T::count) return true;
			else {
				if constexpr (pure::Applicable<U, typename record_nth_id<index, typename T::Id_Elements_List>::value>) {
					if (!eq (apply (other, self.template nth_id<index> ()), self.template nth_element<index> ()))
						return false;
					return record_equal_function<index + 1> (eq, self, other);
				}
				else return false;
			}
		}

		template<typename LHS, typename RHS, intptr_t index = 0>
		constexpr bool record_equal_record_id () {
			if constexpr (index == LHS::count) return true;
			else {
				if constexpr (
						record_id_index<typename record_nth_id<index, typename LHS::Id_Elements_List>::value, typename RHS::Id_Elements_List>::eval () ==
						-1)
					return false;
				else return record_equal_record_id<LHS, RHS, index + 1> ();
			}
		}

		template<intptr_t index = 0, typename Eq, typename LHS, typename RHS>
		bool record_equal_record_elements (const Eq& eq, const LHS& lhs, const RHS& rhs) {
			if constexpr (index == LHS::count) return true;
			else {
				if (!eq (lhs.template nth_element<index> (), rhs (lhs.template nth_id<index> ()))) return false;
				return record_equal_record_elements<index + 1> (eq, lhs, rhs);
			}
		}

		template<typename Eq, typename LHS, typename RHS>
		bool record_equal_record (const Eq& eq, const LHS& lhs, const RHS& rhs) {
			if constexpr (!(std::is_same_v<LHS, RHS> || record_equal_record_id<LHS, RHS> ())) return false;
			else return record_equal_record_elements (eq, lhs, rhs);
		}

		template<typename... Args>
		struct is_record_init_t : std::false_type {};

		template<typename Id, typename Value>
		struct is_record_init_t<Id, Value> { static constexpr bool value = is_static_id<std::decay_t<Id>>::value; };

		template<typename Id, typename Value, typename... Args>
		struct is_record_init_t<Id, Value, Args...> {
			static constexpr bool value = is_static_id<std::decay_t<Id>>::value ? is_record_init_t<Args...>::value
																				: false;
		};

		template<typename... Args>
		static constexpr bool is_record_init = is_record_init_t<Args...>::value;

	}
}

namespace pure {
	template<typename... Id_Elements>
	struct record : implements<Type_Class::Record> {
		static_assert (sizeof... (Id_Elements) > 0);
		static_assert (sizeof... (Id_Elements) % 2 == 0);

		using Id_Elements_List = typename detail::pair_type_list<detail::type_list<Id_Elements...>>::value;
		static constexpr intptr_t count = sizeof... (Id_Elements) / 2;
		using tuple_t = typename detail::record_tuple_t<Id_Elements_List>::value;
		static_assert (detail::tuple::count<tuple_t> == count);

		tuple_t elements;

		template<typename... Args>
		record (Args&& ... args) : elements (std::forward<Args> (args)...) {}

		record (record& other) : elements (other.elements) {};
		record (const record& other) : elements (other.elements) {};
		record (record&& other) : elements (std::move (other.elements)) {};

		template<intptr_t i>
		Interface::Value* nth_id_get (intptr_t n) const {
			if constexpr (i == count) throw operation_not_supported ();
			else {
				if (i == n) return Var::obj (typename detail::record_nth_id<i, Id_Elements_List>::value ());
				else return nth_id_get<i + 1> (n);
			}
		}

		template<intptr_t n>
		auto nth_id () const {
			return typename detail::record_nth_id<n, Id_Elements_List>::value ();
		}

		interned<> nth_id (intptr_t n) const {
			return {intern, nth_id_get<0> (n)};
		}

		template<intptr_t n>
		auto nth_element () const -> decltype (detail::tuple::nth<n> (elements)) {
			return detail::tuple::nth<n> (elements);
		}

		template<intptr_t n>
		auto nth_ref_pair () const {
			return tuple<decltype (nth_id<n> ()), decltype (nth_element<n> ())> {nth_id (), nth_element ()};
		}

		auto nth_element (intptr_t n) const {
			return detail::tuple::generic_nth (elements, n);
		}

		auto
		nth_pair (intptr_t n) const -> tuple<std::decay_t<decltype (nth_id (n))>, std::decay_t<decltype (nth_element (
				n))>> {
			return {nth_id (n), nth_element (n)};
		}

		template<intptr_t n>
		auto
		nth_pair () const -> tuple<std::decay_t<decltype (nth_id<n> ())>, std::decay_t<decltype (nth_element<n> ())>> {
			return {nth_id<n> (), nth_element<n> ()};
		}

		struct enumerator : enumerator_base<tuple<interned<>, var>> {
			const record& owner;
			intptr_t index = 0;
			mutable value_type current;

			enumerator (const record& owner) : owner {owner}, current {owner.nth_id (0), nullptr} {};

			void next () { ++index; }
			bool empty () const noexcept { return index == count; }

			const value_type& read () const {
				detail::tuple::nth<0> (current) = owner.nth_id (index);
				detail::tuple::nth<1> (current) = owner.nth_element (index);

				return current;
			}
			value_type move () const { return read (); }

			bool has_size () const noexcept { return true; }
			bool size () const noexcept { return count - index; }
		};

		template<intptr_t index = 0, typename Id>
		intptr_t get_id_index (const Id& identifier) const {
			if constexpr (index == count) return -1;
			else {
				if (pure::equal (nth_id<index> (), identifier)) return index;
				else return get_id_index<index + 1> (identifier);
			}
		}

		template<typename Id>
		auto operator() (const Id& identifier) const {
			if constexpr (is_static_id<Id>::value) {
				constexpr auto index = detail::record_id_index<Id, Id_Elements_List>::eval ();
				if constexpr (index == -1) throw operation_not_supported ();
				else return nth_element<index> ();
			}
			else {
				auto index = get_id_index (identifier);
				if (index == -1) throw operation_not_supported ();
				else return nth_element (index);
			}
		}

		template<typename Id, typename Value>
		auto set (const Id& identifier, Value&& v) const {
			throw operation_not_supported ();
			return *this;
		}

		enumerator enumerate () const { return *this; }
	};


	template<typename T, intptr_t count, typename First_Id, typename First_Element, typename... Args>
	T make_record_helper (First_Id&& _, First_Element&& first, Args&& ... args) {
		if constexpr (count == 1) {
			return T {std::forward<Args> (args)..., std::forward<First_Element> (first)};
		}
		else {
			return make_record_helper<T, count - 1> (std::forward<Args> (args)..., std::forward<First_Element> (first));
		}
	};

	template<typename... Args>
	auto make_record (Args&& ... args) {
		return make_record_helper<record<std::decay_t<Args>...>, sizeof... (Args) / 2> (std::forward<Args> (args)...);
	}
}
