#pragma once

#include <pure/traits.hpp>
#include <pure/object/interface.hpp>
#include <pure/support/enumerator.hpp>
#include <pure/impl/Trait_Compare.hpp>
#include <pure/impl/Trait_Hash.hpp>
#include <pure/impl/Trait_Print.hpp>
#include <pure/impl/Trait_Type_Variants.hpp>

#include <vector>

namespace pure {
	namespace Basic {
		/**
		 Persistent Vector implementation using immer library. Has constant time persistent append and set
		 @tparam T Type for items in the vector
		 */
		template<typename T>
		struct Vector : Interface::Value {
			using domain_t = Vector_t<pure::domain_t<T>>;
			using element_type = T;
			using vector_type = std::vector<element_type>;
			using iterator_type = typename vector_type::const_iterator;
			vector_type self;

			template<typename Other>
			Vector (Other&& other) : self {} {
				if constexpr (Trait_Enumerable<std::decay_t<Other>>::implemented) {
					for (auto enumerator = pure::enumerate (
							std::forward<Other> (other)); !enumerator.empty (); enumerator.next ()) {
						self.emplace_back (enumerator.move ());
					}
				}
				else {
					static_assert (detail::not_reachable<Other>);
				}
			}

			template<typename... Args>
			void append_elements (Args&& ... args) {}

			template<typename First, typename... Args>
			void append_elements (First&& first, Args&& ... args) {
				self.emplace_back (std::forward<First> (first));
				return append_elements (std::forward<Args> (args)...);
			}

			template<typename... Args>
			Vector (init_tag, Args&& ... args) : self () {
				self.reserve (sizeof... (args));
				append_elements (std::forward<Args> (args)...);
			}

			Vector (Vector& other) : self (other.self) {};
			Vector (const Vector& other) : self (other.self) {};
			Vector (Vector&& other) : self (std::move (other.self)) {};

			Vector (const vector_type& other) : self (other) {};
			Vector (vector_type&& other) : self (std::move (other)) {};

			int category_id () const noexcept override { return Any_Vector.id; }

			Interface::Value* clone () const& override { return new Vector {*this}; }
			Interface::Value* clone ()&& override { return new Vector {std::move (*this)}; }
			intptr_t clone_bytes_needed () const override { return sizeof (Vector); }
			Interface::Value* clone_placement (void* memory, intptr_t num_bytes) const& override {
				return new (memory) Vector {*this};
			}
			Interface::Value* clone_placement (void* memory, intptr_t num_bytes)&& override {
				return new (memory) Vector {std::move (*this)};
			}

			bool equal (const weak<>& other) const override {
				return detail::equal_sequence (pure::equal, enumerate (), other);
			}

			bool equivalent (const weak<>& other) const override {
				return detail::equal_sequence (pure::equivalent, enumerate (), other);
			}

			int compare (const weak<>& other) const override {
				return detail::compare_sequence (pure::compare, enumerate (), other);
			}

			int equivalent_compare (const weak<>& other) const override {
				return detail::compare_sequence (pure::equivalent_compare, enumerate (), other);
			}

			int32_t hash () const override {
				return detail::hash_sequence (enumerate ());
			}

			const element_type& apply (intptr_t n) const { return self[n]; }
			var virtual_apply (const var& n) const override { return apply (n); }

			intptr_t arity () const noexcept override { return 1; }
			bool Variadic () const noexcept override { return false; }

			template<typename Index, typename Value>
			auto set_persistent (const var&, const Index& index, Value&& value) const {
				auto i = static_cast<typename vector_type::size_type>(index);
				if constexpr (std::is_convertible_v<Value&&, element_type>) { // TODO Check for unification
					auto copy {self};
					copy[i] = (std::forward<Value> (value)); // TODO Don't copy reset element
					return immediate<Vector> {std::move (copy)};
				}
				else {
					std::vector<var> copy;

					for (auto iter = self.begin (); iter != self.begin () + index; ++iter) {
						copy.emplace_back (*iter);
					}
					copy.emplace_back (std::forward<Value> (value));
					for (auto iter = self.begin () + index + 1; iter != self.end (); ++iter) {
						copy.emplace_back (*iter);
					}

					return immediate<Vector<var>> {std::move (copy)};
				}
			};

			some<> virtual_set_persistent (const var&, var&& key, var&& value) const override {
				return set_persistent ({}, key, std::move (value));
			}

			template<typename Index, typename Value>
			auto set_transient (var&&, const Index& index, Value&& value) {
				auto i = static_cast<typename vector_type::size_type>(index);
				if constexpr (std::is_convertible_v<Value&&, element_type>) {
					self[i] = (std::forward<Value> (value));
					return immediate<Vector> {std::move (self)};
				}
				else {
					std::vector<var> copy;

					for (auto iter = self.begin (); iter != self.begin () + index; ++iter) {
						copy.emplace_back (std::move (*iter));
					}
					copy.emplace_back (std::forward<Value> (value));
					for (auto iter = self.begin () + index + 1; iter != self.end (); ++iter) {
						copy.emplace_back (std::move (*iter));
					}

					return immediate<Vector<var>> {std::move (copy)};
				}
			};

			maybe<> virtual_set_transient (var&&, var&& key, var&& value) override {
				return set_transient ({}, key, std::move (value));
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

			iterator_range<iterator_type> enumerate () const { return {self.begin (), self.end ()}; }
			generic_enumerator virtual_enumerate () const override { return enumerate (); }

			intptr_t count () const noexcept override { return self.size (); }
			bool Empty () const noexcept override { return self.empty (); }

			const element_type& first () const { return nth (0); }
			var virtual_first () const override { return first (); }

			const element_type& second () const { return nth (0); }
			var virtual_second () const override { return second (); }

			const element_type& nth (intptr_t n) const { return self[n]; }
			var virtual_nth (intptr_t n) const override { return nth (n); }

			template<typename Value>
			auto append_persistent (const var&, Value&& value) const {
				if constexpr (std::is_convertible_v<Value&&, element_type>) {
					auto copy {self};
					copy.emplace_back (std::forward<Value> (value));
					return immediate<Vector> {std::move (copy)};
				}
				else {
					std::vector<var> copy;
					copy.reserve (self.size () + 1);
					for (const auto& element : self) {
						copy.emplace_back (element);
					}
					copy.emplace_back (std::forward<Value> (value));
					return immediate<Vector<var>> {std::move (copy)};
				}
			}

			some<> virtual_append_persistent (const var& self, var&& element) const override {
				return append_persistent ({}, std::move (element));
			};

			template<typename Value>
			auto append_transient (const var&, Value&& value) {
				if constexpr (std::is_convertible_v<Value&&, element_type>) {
					self.emplace_back (std::forward<Value> (value));
					return immediate<Vector> {std::move (self)};
				}
				else {
					std::vector<var> copy;
					copy.reserve (self.size () + 1);
					for (auto& element : self) {
						copy.emplace_back (std::move (element));
					}

					copy.emplace_back (std::forward<Value> (value));
					return immediate<Vector<var>> {std::move (copy)};
				}
			}
			maybe<> virtual_append_transient (var&& self, var&& element) override {
				return append_transient ({}, std::move (element));
			};

			template<typename Stream>
			void print_to (Stream& stream) const {
				detail::print_sequence_to (stream, enumerate ());
			}
			void virtual_print_to (var& stream) const override { this->print_to (stream); }

		};

	}

	template<typename T = var, typename... Args>
	immediate<Basic::Vector<T>> make_vector (Args&& ... args) {
		return {init, std::forward<Args> (args)...};
	}
}
