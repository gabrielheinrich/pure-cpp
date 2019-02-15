#pragma once

#include <pure/traits.hpp>
#include <pure/object/interface.hpp>
#include <pure/support/enumerator.hpp>
#include <pure/impl/Trait_Compare.hpp>
#include <pure/impl/Trait_Hash.hpp>
#include <pure/impl/Trait_Print.hpp>

#include <pure/support/immer_compat.hpp>
#include <immer/map.hpp>

namespace pure {
	struct equal_t {
		template<typename LHS, typename RHS>
		bool operator() (const LHS& lhs, const RHS& rhs) { return pure::equal (lhs, rhs); }
	};

	struct hash_t {
		template<typename T>
		int32_t operator() (const T& self) { return pure::hash (self); }
	};
}

namespace pure {
	/**
	 * Persistent Datastructures
	 */
	namespace Persistent {
		/**
		 Persistent Map implementation using immer library. Has constant time persistent set
		 @tparam Key Type for keys
		 @tparam Val Type for values
		 */
		template<typename Key, typename Val>
		struct Map : Interface::Value {
			using domain_t = Function_t<pure::domain_t<Key>, pure::domain_t<Val>>;
			using key_type = Key;
			using value_type = Val;
			using map_type = immer::map<key_type, value_type, hash_t, equal_t>;
			using pair_type = typename map_type::value_type;
			using iterator_type = typename map_type::const_iterator;
			map_type self;

			template<typename Other>
			Map (Other&& other) : self {} {
				if constexpr (Trait_Enumerable<std::decay_t<Other>>::implemented) {
					if (pure::category_id (other) != Any_Function.id) throw operation_not_supported ();

					for (auto enumerator = pure::enumerate (
							std::forward<Other> (other)); !enumerator.empty (); enumerator.next ()) {
						auto&& element = enumerator.move ();
						self = std::move (self).set (pure::first (std::move (element)),
													 pure::second (std::move (element)));
					}
				}
				else {
					static_assert (detail::not_reachable<Other>);
				}
			}

			template<typename... Args>
			void insert_element (Args&& ... args) {}

			void insert_elements() {}

			template<typename K, typename Va, typename... Args>
			void insert_elements (K&& key, Va&& value, Args&& ... args) {
				self = std::move (self).set (std::forward<K> (key), std::forward<Va> (value));
				insert_elements (std::forward<Args>(args)...);
			};

			template<typename... Args>
			Map (init_tag, Args&& ... args) : self {} {
				insert_elements (std::forward<Args> (args)...);
			}

			Map (Map& other) : self {other.self} {};
			Map (const Map& other) : self {other.self} {};
			Map (Map&& other) : self {std::move (other.self)} {};

			Map (const map_type& other) : self {other} {};
			Map (map_type&& other) : self {std::move (other)} {};

			int category_id () const noexcept override { return Any_Function.id; }

			Interface::Value* clone () const& override { return new Map {*this}; }
			Interface::Value* clone ()&& override { return new Map {std::move (*this)}; }
			intptr_t clone_bytes_needed () const override { return sizeof (Map); }
			Interface::Value* clone_placement (void* memory, intptr_t num_bytes) const& override {
				return new (memory) Map {*this};
			}
			Interface::Value* clone_placement (void* memory, intptr_t num_bytes)&& override {
				return new (memory) Map {std::move (*this)};
			}

			bool equal (const weak<>& other) const override {
				return detail::equal_map (pure::equal, enumerate (), count (), other);
			}

			bool equivalent (const weak<>& other) const override {
				return detail::equal_map (pure::equivalent, enumerate (), count (), other);
			}

			int32_t hash () const override {
				return detail::hash_map (enumerate ());
			}

			template<typename K>
			const value_type& apply (const K& key) const { return self.at (key); }
			var virtual_apply (const any& key) const override { return apply (key); }

			intptr_t arity () const noexcept override { return 1; }
			bool Variadic () const noexcept override { return false; }

			template<typename K, typename V>
			immediate<Map> set_persistent (const any&, K&& key, V&& value) const {
				return self.set (std::forward<K> (key), std::forward<V> (value));
			};
			some<> virtual_set_persistent (const any&, any&& key, any&& value) const override {
				return set_persistent ({}, std::move (key), std::move (value));
			}

			template<typename K, typename V>
			immediate<Map> set_transient (const any&, K&& key, V&& value) {
				return std::move (self).set (std::forward<K> (key), std::forward<V> (value));
			};

			maybe<> virtual_set_transient (any&&, any&& key, any&& value) override {
				return set_transient ({}, std::move (key), std::move (value));
			}

			struct enumerator : enumerator_base<typename iterator_type::value_type> {
				using value_type = typename iterator_type::value_type;

				iterator_type iterator;
				intptr_t count;
				intptr_t index;

				enumerator (iterator_type iterator, intptr_t count) : iterator {iterator}, count {count}, index {0} {}

				bool empty () const noexcept { return index == count; }
				void next () {
					++iterator;
					++index;
				}

				const value_type& read () const { return *iterator; }
				value_type move () { return *iterator; }

				bool has_size () const noexcept { return true; }
				intptr_t size () const noexcept { return count - index; }
			};

			bool Enumerable () const noexcept override { return true; }

			enumerator enumerate () const { return {self.begin (), count ()}; }
			generic_enumerator virtual_enumerate () const override { return enumerate (); }

			intptr_t count () const noexcept override { return self.size (); }
			bool Empty () const noexcept override { return count () == 0; }

			const pair_type& first () const { return nth (0); }
			var virtual_first () const override { return first (); }

			const pair_type& second () const { return nth (0); }
			var virtual_second () const override { return second (); }

			const pair_type& nth (intptr_t n) const {
				auto iter = self.begin ();
				while (n-- > 0) ++iter;
				return *iter;
			}
			var virtual_nth (intptr_t n) const override { return nth (n); }

			template<typename Stream>
			void print_to (Stream& stream) const {
				detail::print_map_to (stream, enumerate ());
			}
			void virtual_print_to (any& stream) const override { this->print_to (stream); }

		};

		template<typename... Args>
		immediate<Map<var, var>> make_map (Args&& ... args) {
			static_assert (sizeof... (Args) % 2 == 0);
			return {init, std::forward<Args> (args)...};
		};
	}
}
