#pragma once

#include <pure/types/any.hpp>
#include <pure/types/unique.hpp>
#include <pure/object/interface.hpp>
#include <pure/traits.hpp>
#include <pure/support/misc.hpp>
#include <pure/support/utf8.hpp>
#include <pure/impl/Trait_Value.hpp>
#include <pure/impl/Trait_Hash.hpp>
#include <pure/impl/Trait_From_Var.hpp>
#include <pure/support/enumerator.hpp>
#include <pure/impl/Trait_Print.hpp>

namespace pure {
	struct with_capacity_t {};
	static constexpr with_capacity_t with_capacity {};

	struct cstring_enumerator : enumerator_base<char32_t> {
		const char* position;

		cstring_enumerator (const char* cstring) : position {cstring} {}
		cstring_enumerator (const cstring_enumerator&) = default;

		void next () noexcept { position = utf8::advance (position); }
		bool empty () const noexcept { return *position == '\0'; }
		char32_t read () const noexcept { return utf8::read_char (position); }
		char32_t move () const noexcept { return read (); }
	};
}

namespace pure {
	/**
	 * Basic Builtin Types
	 */
	namespace Basic {
		/**
		 Default string implementation.
		 */
		struct String : Interface::Value {
			static char32_t cstring_nth (const char* cstring, intptr_t n) {
				auto pos = utf8::advance_n (cstring, n);
				if (pos) return utf8::read_char (pos);
				else throw operation_not_supported ();
			}

			using domain_t = String_t;

			intptr_t num_allocated_bytes;
			char* str_end;
			char str[1];

			static constexpr bool has_dynamic_size = true;

			static intptr_t capacity_needed_for_length (intptr_t length) { return detail::round_up_8 (length + 1) - 1; }
			template<typename T>
			static constexpr intptr_t capacity_needed (const T& other) {
				return capacity_needed_for_length (pure::raw_cstring_length (other));
			}

			template<typename... Args>
			static constexpr intptr_t capacity_needed (with_capacity_t, intptr_t capacity, const Args& ... args) {
				return capacity;
			}

			static constexpr intptr_t capacity_to_num_bytes (intptr_t capacity) {
				return sizeof (String) + capacity + 1;
			}

			static unique<String> create_empty (intptr_t capacity) {
				auto num_bytes = capacity_to_num_bytes (capacity);
				return new (sized_allocation, num_bytes) String {capacity};
			}

			static unique<String> create_from_cstring (intptr_t capacity, const char* str, intptr_t strlen = -1) {
				return new (sized_allocation, capacity_to_num_bytes (capacity)) String {capacity, str, strlen};
			}

			intptr_t capacity () const noexcept { return num_allocated_bytes - 1; }
			String (intptr_t capacity) : num_allocated_bytes {capacity + 1}, str_end {str} {
				str[0] = '\0';
			}

			intptr_t remaining_capacity () const noexcept { return capacity () - cstring_length (); }

			void append_cstring (const char* str, intptr_t num_bytes) {
				assert (remaining_capacity () >= num_bytes);
				std::memcpy (str_end, str, num_bytes);
				str_end += num_bytes;
				*str_end = '\0';
			}
			void append_char (char32_t c) {
				assert (remaining_capacity () >= utf8::bytes_required_for (c));
				str_end = utf8::write_char (str_end, c);
				*str_end = '\0';
			}

			template<typename T>
			String (intptr_t _, with_capacity_t, intptr_t capacity, T&& other) : String (capacity,
																						 pure::raw_cstring (other),
																						 pure::raw_cstring_length (
																								 other)) {}

			String (intptr_t capacity, const char* cstr, intptr_t cstr_len = -1) : num_allocated_bytes {capacity + 1} {
				if (cstr_len == -1)
					cstr_len = std::strlen (cstr);
				assert (cstr_len <= capacity);
				std::memcpy (str, cstr, cstr_len);
				str_end = str + cstr_len;
				*str_end = '\0';
			}

			template<typename T>
			String (intptr_t capacity, T&& other) : String (capacity, pure::raw_cstring (other),
															pure::raw_cstring_length (other)) {}

			int category_id () const noexcept override { return pure::String.id; }

			Interface::Value* clone () const& override {
				return create_from_cstring (capacity_needed_for_length (cstring_length ()), str,
											cstring_length ()).release ();
			}

			intptr_t clone_bytes_needed () const override {
				return capacity_to_num_bytes (capacity_needed_for_length (cstring_length ()));
			}

			Interface::Value* clone_placement (void* memory, intptr_t num_bytes) const& override {
				assert (num_bytes == clone_bytes_needed ());
				return create_from_cstring (capacity_needed_for_length (cstring_length ()), str,
											cstring_length ()).release ();
			}

			const char* cstring () const override {
				return str;
			}

			intptr_t cstring_length () const override {
				return str_end - str;
			}

			bool equal (const weak<>& other) const override {
				if (pure::category_id (other) == pure::String.id) {
					return pure::equal (str, other->cstring ());
				}
				return false;
			}

			bool equivalent (const weak<>& other) const override {
				if (pure::category_id (other) == pure::String.id) {
					return pure::equal (str, other->cstring ());
				}
				return false;
			}

			int compare (const weak<>& other) const override {
				return pure::compare (str, other);
			}

			int equivalent_compare (const weak<>& other) const override {
				return pure::equivalent_compare (str, other);
			}

			int32_t hash () const override {
				return detail::hash_cstring_with_length (cstring (), cstring_length ());
			}

			char32_t apply (intptr_t index) const {
				return cstring_nth (cstring (), index);
			};

			var virtual_apply (const any& a0) const override { return apply (a0); }

			intptr_t arity () const noexcept override { return 1; }
			bool Variadic () const noexcept override { return false; }

			unique<String> set_persistent (const any&, intptr_t index, char32_t c) const {
				const char* pos = str;
				for (intptr_t i = 0; i < index; ++i) {
					if (*pos == '\0') throw operation_not_supported (); // TODO out of bounds
					pos = utf8::advance (pos);
				}

				const char* tail_begin = *pos == '\0' ? pos : utf8::advance (pos);

				auto result = create_from_cstring (capacity_needed_for_length (cstring_length () + 4), this->str,
												   pos - this->str);
				result->append_char (c);
				result->append_cstring (tail_begin, this->str_end - tail_begin);
				return std::move (result);
			};
			some<> virtual_set_persistent (const any&, any&& key, any&& value) const override {
				return set_persistent ({}, key, value);
			}

			unique<String, maybe_nil> set_transient (any&&, intptr_t index, char32_t c) {
				char* pos = str;
				for (intptr_t i = 0; i < index; ++i) {
					if (*pos == '\0') throw operation_not_supported (); // TODO out of bounds
					pos = utf8::advance (pos);
				}

				char* tail_begin = *pos == '\0' ? pos : utf8::advance (pos);

				intptr_t num_bytes_current_char = tail_begin - pos;
				intptr_t num_bytes_new_char = utf8::bytes_required_for (c);

				if (num_bytes_new_char - num_bytes_current_char > remaining_capacity ()) {
					auto result = create_from_cstring (capacity_needed_for_length (cstring_length () * 2 + 4),
													   this->str, pos - this->str);
					result->append_char (c);
					result->append_cstring (tail_begin, this->str_end - tail_begin);
					return std::move (result);
				}

				char* shift_position = pos + num_bytes_new_char;

				auto new_str_end = str_end + (shift_position - tail_begin);

				if (shift_position != tail_begin) {
					std::memmove (shift_position, tail_begin, str_end - tail_begin + 1);
				}
				this->str_end = new_str_end;
				utf8::write_char (pos, c);
				return nullptr;
			};

			maybe<> virtual_set_transient (any&&, any&& key, any&& value) override {
				return set_transient ({}, key, value);
			}

			bool Enumerable () const noexcept override { return true; }

			cstring_enumerator enumerate () const noexcept { return {cstring ()}; }
			generic_enumerator virtual_enumerate () const override { return enumerate (); }

			intptr_t count () const override { return utf8::count (cstring ()); }
			bool Empty () const override { return *cstring () == '\0'; }

			char32_t first () const { return cstring_nth (cstring (), 0); }
			var virtual_first () const override { return first (); }

			char32_t second () const { return cstring_nth (cstring (), 1); }
			var virtual_second () const override { return second (); }

			char32_t nth (intptr_t n) const { return cstring_nth (cstring (), n); }
			var virtual_nth (intptr_t n) const override { return nth (n); }

			unique<String> append_persistent (const any&, char32_t c) const {
				auto new_capacity = capacity_needed_for_length (cstring_length () + 32);
				auto result = create_from_cstring (new_capacity, cstring (), cstring_length ());
				result->append_char (c);
				return std::move (result);
			}
			some<> virtual_append_persistent (const any& self, any&& element) const override {
				return append_persistent ({}, std::move (element));
			};

			unique<String, maybe_nil> append_transient (any&&, char32_t c) {
				if (remaining_capacity () >= utf8::bytes_required_for (c)) {
					this->append_char (c);
					return nullptr;
				}
				else return append_persistent ({}, c);
			}

			maybe<> virtual_append_transient (any&& self, any&& element) override {
				return append_transient ({}, std::move (element));
			};

			template<typename Stream>
			void print_to (Stream& stream) const {
				IO::print_to (stream, cstring ());
			}
			void virtual_print_to (any& stream) const override { this->print_to (stream); }
		};
	}
}
