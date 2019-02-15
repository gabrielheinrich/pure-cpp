#pragma once

#include <pure/traits.hpp>

namespace pure {
	namespace detail {
		namespace tuple {
			template<typename T>
			constexpr intptr_t count = std::tuple_size<T>::value;

			template<intptr_t n, typename T>
			constexpr auto&& nth (T&& self) {
				if constexpr (is_moveable<T&&>)
					return std::move (std::get<n> (self));
				else
					return std::get<n> (self);
			};

			template<typename T, typename init_type, typename function_type, intptr_t index = 0>
			auto reduce (const T& self, init_type&& init, const function_type& function) {
				if constexpr (tuple::count<T> == index) return init;
				else
					return tuple::reduce<T, init_type, function_type, index + 1> (self, function (init,
																								  tuple::nth<index> (
																										  self)),
																				  function);
			};


			template<intptr_t index, intptr_t count, typename LHS, typename RHS>
			bool equal_enumerator (const LHS& lhs, RHS& rhs) {
				if constexpr (index == count) return rhs.empty ();
				else {
					if (!pure::equal (nth<index> (lhs), rhs.read ())) return false;
					else {
						rhs.next ();
						return equal_enumerator<index + 1, count> (lhs, rhs);
					}
				}
			};

			template<intptr_t index, intptr_t count, typename LHS, typename RHS>
			bool equivalent_enumerator (const LHS& lhs, RHS& rhs) {
				if constexpr (index == count) return rhs.empty ();
				else {
					if (!pure::equivalent (nth<index> (lhs), rhs.read ())) return false;
					else {
						rhs.next ();
						return equivalent_enumerator<index + 1, count> (lhs, rhs);
					}
				}
			};

			template<intptr_t index, intptr_t count, typename LHS, typename RHS>
			int compare_enumerator (const LHS& lhs, RHS& rhs) {
				if constexpr (index == count) return rhs.empty () ? 0 : -1;
				else {
					if (int comparison = pure::compare (tuple::nth<index> (lhs), rhs.read ());
					comparison) return comparison;
					else {
						rhs.next ();
						return compare_enumerator<index + 1, count> (lhs, rhs);
					}
				}
			};

			template<intptr_t index, intptr_t count, typename LHS, typename RHS>
			int equivalent_compare_enumerator (const LHS& lhs, RHS& rhs) {
				if constexpr (index == count) return rhs.empty () ? 0 : -1;
				else {
					if (int comparison = pure::equivalent_compare (nth<index> (lhs), rhs.read ());
					comparison) return comparison;
					else {
						rhs.next ();
						return equivalent_compare_enumerator<index + 1, count> (lhs, rhs);
					}
				}
			};

			template<intptr_t index, intptr_t count, typename LHS, typename RHS, typename Eq>
			bool do_equal_loop (const LHS& lhs, const RHS& rhs, const Eq& eq) {
				if constexpr (index == count) return true;
				else {
					if (!eq (tuple::nth<index> (lhs), tuple::nth<index> (rhs))) return false;
					else return tuple::do_equal_loop<index + 1, count> (lhs, rhs, eq);
				}
			};

			template<typename LHS, typename RHS, typename Eq>
			bool do_equal (const LHS& lhs, const RHS& rhs, const Eq& eq) {
				constexpr auto count = tuple::count<LHS>;
				if constexpr (count != tuple::count<RHS>) return false;
				else return tuple::do_equal_loop<0, count> (lhs, rhs, eq);
			};

			template<intptr_t index, intptr_t lhs_count, intptr_t rhs_count, typename LHS, typename RHS, typename Comparator>
			int do_compare (const LHS& lhs, const RHS& rhs, const Comparator& comp) {
				if constexpr (index == lhs_count) return index == rhs_count ? 0 : -1;
				else if constexpr (index == rhs_count) return 1;
				else {
					if (int comparison = comp (nth<index> (lhs), nth<index> (rhs));
					comparison) {
						return comparison;
					}
					else return do_compare<index + 1, lhs_count, rhs_count> (lhs, rhs, comp);

				}
			};

			template<typename... Rest>
			struct common_ref_type_t { using value = void; };

			template<typename First>
			struct common_ref_type_t<First> { using value = First; };

			template<typename First, typename Second, typename... Rest>
			struct common_ref_type_t<First, Second, Rest...> {
				using value = std::conditional_t<std::is_base_of_v<First, Second>, typename common_ref_type_t<First, Rest...>::value,
						std::conditional_t<std::is_base_of_v<Second, First>, typename common_ref_type_t<Second, Rest...>::value, void>
				>;
			};

			template<typename... Ts>
			using common_ref_type = typename common_ref_type_t<Ts...>::value;

			template<typename... Ts>
			constexpr bool common_ref_type_exists = !std::is_void_v<common_ref_type<Ts...>>;

			template<typename T, typename Type_Class = type_class<T>>
			struct generic_nth_impl;

			template<typename return_type, intptr_t index, intptr_t count, typename T>
			return_type generic_nth_get (T&& self, intptr_t n) {
				if constexpr (index < count) {
					if (n == index) return tuple::nth<index> (std::forward<T> (self));
					else return generic_nth_get<return_type, index + 1, count> (std::forward<T> (self), n);
				}
				else throw operation_not_supported (); // TODO : out_of_bounds
			};

			template<typename T, typename... Elements>
			struct generic_nth_impl<T, Type_Class::Tuple<Elements...>> {
				using common_ref = common_ref_type<Elements...>;
				static constexpr bool has_common_ref = !std::is_void_v<common_ref>;
				using return_type = std::conditional_t<has_common_ref, common_ref, pure::unify_types<Elements...>>;

				template<typename TT>
				static return_type eval (TT&& self, intptr_t n) {
					return generic_nth_get<return_type, 0, sizeof... (Elements)> (std::forward<TT> (self), n);
				}
			};

			template<typename T>
			auto generic_nth (T&& self, intptr_t n) {
				return generic_nth_impl<std::decay_t<T>, type_class<T>>::eval (std::forward<T> (self), n);
			}

			template<typename T, typename Type_Class = type_class<T>>
			struct common_element_type_t;

			template<typename T, typename... Elements>
			struct common_element_type_t<T, Type_Class::Tuple<Elements...>> {
				using value = unify_types<Elements...>;
			};

			template<intptr_t count, typename T, typename... Ts>
			struct tuple_of_t {
				static constexpr auto eval () {
					if constexpr (sizeof... (Ts) == count - 1)
						return type_value<std::tuple<T, Ts...>> {};
					else
						return tuple_of_t<count, T, T, Ts...>::eval ();
				}
				using value = typename decltype (tuple_of_t<count, T, Ts...>::eval ())::value;
			};

			template<typename return_type, intptr_t index, intptr_t count>
			struct do_set_n {
				template<typename T, typename Value>
				static return_type eval (T&& self, Value&& value) {
					if constexpr (count == 1) {
						return {std::forward<Value> (value)};
					}
					else if constexpr (count == 2) {
						if constexpr (index == 0) {
							return {std::forward<Value> (value), tuple::nth<1> (std::forward<T> (self))};
						}
						else {
							return {tuple::nth<0> (std::forward<T> (self)), std::forward<Value> (value)};
						}
					}
					else {
						return {};
					}
				}
			};

			template<typename return_type, intptr_t index, intptr_t count, typename T, typename Value>
			return_type do_set (T&& self, intptr_t n, Value&& value) {
				if constexpr (index < count) {
					if (index == n)
						return do_set_n<return_type, index, count>::eval (std::forward<T> (self),
																		  std::forward<Value> (value));
					else
						return do_set<return_type, index + 1, count> (std::forward<T> (self), n,
																	  std::forward<Value> (value));
				}
				else
					throw operation_not_supported ();
			};

			template<typename T, typename Value>
			auto set (T&& self, intptr_t index, Value&& value) {
				constexpr auto count = tuple::count<T>;
				using element_type = unify_types<std::decay_t<Value>, typename common_element_type_t<T>::value>;
				using return_type = typename tuple_of_t<count, element_type>::value;
				if (index >= count) throw operation_not_supported ();
				else {
					return do_set<return_type, 0, count> (std::forward<T> (self), index, std::forward<Value> (value));
				}
			};

			template<typename T, typename... Elements>
			struct enumerator {
				static constexpr intptr_t count = sizeof... (Elements);
				using common_ref_type = tuple::common_ref_type<Elements...>;
				static constexpr bool has_ref = std::is_void_v<common_ref_type> == false;
				using value_type = std::conditional_t<has_ref, common_ref_type, unify_types<Elements...>>;
				using read_type = std::conditional_t<has_ref, const value_type&, value_type>;

				intptr_t index = 0;
				const T& self;
				enumerator (const T& self) : self {self} {};

				bool empty () const noexcept { return index == count; }
				void next () { ++index; }

				read_type read () const { return generic_nth_get<read_type, 0, count> (self, index); }
				value_type move () const { return generic_nth_get<value_type, 0, count> (self, index); }

				constexpr bool has_size () const noexcept { return true; }
				constexpr intptr_t size () const noexcept { return count; }
			};

			template<typename T, typename V>
			struct tuple_add_t;

			template<typename V, typename... Elements>
			struct tuple_add_t<Type_Class::Tuple<Elements...>, V> { using value = std::tuple<Elements..., std::decay_t<V>>; };

			template<typename T, typename Value>
			auto append (T&& self, Value&& value) {
				return std::tuple<std::decay_t<Value>> {std::forward<Value> (value)};
			};
		}
	}
}

namespace pure {
	using std::tuple;

	template<typename... Args>
	auto make_tuple (Args&& ... args) -> tuple<std::decay_t<Args>...> {
		return {std::forward<Args> (args)...};
	}
}
