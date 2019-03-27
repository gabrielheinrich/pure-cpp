#pragma once

#include <pure/symbolic_sets.hpp>
#include <pure/traits.hpp>
#include <pure/exceptions.hpp>
#include <pure/support/ref_count.hpp>

namespace pure {
	struct never_nil;
	struct maybe_nil;

	template<typename O, typename Nil>
	struct some;

	template<typename O, typename Nil>
	struct weak;

	struct var;

	struct generic_enumerator;
}

namespace pure {
	/**
	 * Interface Types
	 */
	struct sized_allocation_t {};
	constexpr sized_allocation_t sized_allocation {};

	namespace Interface {
		/**
		 Base class for all values, which can be held by value holders. Contains a maybe unused reference count.
		 */
		struct alignas (16) Value : detail::ref_counted {
			using domain_t = Any_t;

			static constexpr bool has_dynamic_size = false;

			template<typename T>
			static constexpr intptr_t capacity_needed (const T& other) { return Var::clone_bytes_needed (other); }
			static constexpr intptr_t capacity_to_num_bytes (intptr_t x) { return x; }

			void* operator new (std::size_t count) { return ::operator new (count, std::align_val_t {16}); }
			void operator delete (void* p) {::operator delete (p, std::align_val_t {16});}

			void* operator new (std::size_t, void* p) { return p; }
			void operator delete (void* p, void*) {::operator delete (p, std::align_val_t {16});}

			void* operator new (std::size_t, sized_allocation_t, std::size_t num_bytes) { return ::operator new (num_bytes, std::align_val_t {16}); }
			void operator delete (void* p, sized_allocation_t, std::size_t num_bytes) {::operator delete (p, num_bytes, std::align_val_t {16});}

			void* operator new (std::size_t, std::size_t num_bytes) { return ::operator new (num_bytes, std::align_val_t {16}); }
			void operator delete (void* p, std::size_t) {::operator delete (p, std::align_val_t {16});}

			virtual ~Value () {};
			constexpr Value () = default;
			constexpr Value (const Value&) = delete;

			template<typename T>
			Value (intptr_t capacity, T&& other) : ref_counted {} {
				Var::clone_placement (std::forward<T> (other), this, capacity_to_num_bytes (capacity));
			}

			virtual int category_id () const noexcept { return pure::Object.id; }

			virtual Value* clone () const& { throw operation_not_supported (); }
			virtual Value* clone ()&& { return clone (); }
			virtual intptr_t clone_bytes_needed () const { throw operation_not_supported (); }
			virtual Value*
			clone_placement (void* memory, intptr_t num_bytes) const& { throw operation_not_supported (); }
			virtual Value* clone_placement (void* memory, intptr_t num_bytes)&& {
				return clone_placement (memory, num_bytes);
			}

			virtual const char* cstring () const { throw operation_not_supported (); }
			virtual intptr_t cstring_length () const { throw operation_not_supported (); }

			virtual bool equal (const weak<Value, never_nil>& other) const { throw operation_not_supported (); }
			virtual bool equivalent (const weak<Value, never_nil>& other) const { return equal (other); }
			virtual int compare (const weak<Value, never_nil>& other) const { throw operation_not_supported (); }
			virtual int equivalent_compare (const weak<Value, never_nil>& other) const { return compare (other); }

			virtual int32_t hash () const { throw operation_not_supported (); }

			virtual var virtual_apply () const { throw operation_not_supported (); }
			virtual var virtual_apply (const var&) const { throw operation_not_supported (); }
			virtual var virtual_apply (const var&, const var&) const { throw operation_not_supported (); }
			virtual var virtual_apply (const var&, const var&, const var&) const { throw operation_not_supported (); }

			template<typename... Args>
			var apply (Args&& ... args) const { return virtual_apply (std::forward<Args> (args)...); }

			virtual intptr_t arity () const { throw operation_not_supported (); }
			virtual bool Variadic () const noexcept { return false; }

			virtual some<Value, never_nil> virtual_set_persistent (const var& self, var&& key, var&& value) const;
			virtual some<Value, maybe_nil> virtual_set_transient (var&& self, var&& key, var&& value);

			template<typename... Args>
			some<Value, never_nil> set_persistent (const var& self, Args&& ... args) const;
			template<typename... Args>
			some<Value, maybe_nil> set_transient (var&& self, Args&& ... args);

			virtual some<Value, never_nil> virtual_without_persistent (const var& self, const var& key) const;
			virtual some<Value, maybe_nil> virtual_without_transient (var&& self, const var& key);

			some<Value, never_nil> without_persistent (const var& self, const var& key) const;
			some<Value, maybe_nil> without_transient (var&& self, const var& key);

			virtual bool Enumerable () const noexcept { return false; }
			virtual generic_enumerator virtual_enumerate () const;
			generic_enumerator enumerate () const;

			virtual intptr_t count () const { throw operation_not_supported (); }
			virtual bool Empty () const { throw operation_not_supported (); }

			virtual var virtual_first () const;
			var first () const;

			virtual var virtual_second () const;
			var second () const;

			virtual var virtual_nth (intptr_t n) const;
			var nth (intptr_t n) const;

			virtual some<Value, never_nil> virtual_append_persistent (const var& self, var&& element) const;
			virtual some<Value, maybe_nil> virtual_append_transient (var&& self, var&& element);

			template<typename Arg>
			some<Value, never_nil> append_persistent (const var& self, Arg&& arg) const;
			template<typename Arg>
			some<Value, maybe_nil> append_transient (var&& self, Arg&& arg);

			// Error
			virtual const char* error_message () const { throw operation_not_supported (); }

			// Formatting
			virtual void virtual_print_to (var& stream) const { throw operation_not_supported (); }
			template<typename Stream>
			void print_to (Stream& stream) const;

			// Stream
			virtual bool write_raw_bytes (const uint8_t* data, intptr_t size) { throw operation_not_supported (); }
			virtual bool write_raw_string (const char* data) { throw operation_not_supported (); }
			virtual bool write_raw_byte (uint8_t c) { throw operation_not_supported (); }
			virtual bool vprintf (const char* format, va_list& args) { throw operation_not_supported (); }
		};
	}
}

#include <pure/types/var.hpp>
#include <pure/types/some.hpp>
#include <pure/types/maybe.hpp>
#include <pure/support/enumerator.hpp>
#include <pure/types/immediate.hpp>
#include <pure/object/boxed.hpp>

namespace pure::Interface {
	inline some<Value, never_nil>
	Value::virtual_set_persistent (const var& self, var&& key, var&& value) const { throw operation_not_supported (); }
	inline some<Value, maybe_nil> Value::virtual_set_transient (var&& self, var&& key, var&& value) {
		return virtual_set_persistent (self, std::move (key), std::move (value));
	}

	template<typename... Args>
	inline some<Value, never_nil> Value::set_persistent (const var& self, Args&& ... args) const {
		return virtual_set_persistent (self, detail::move_or_copy (
				std::forward<Args> (args))...);
	}
	template<typename... Args>
	inline some<Value, maybe_nil> Value::set_transient (var&& self, Args&& ... args) {
		return virtual_set_transient (std::move (self), detail::move_or_copy (std::forward<Args> (args))...);
	}

	inline some<Value, never_nil>
	Value::virtual_without_persistent (const var& self, const var& key) const { throw operation_not_supported (); }
	inline some<Value, maybe_nil>
	Value::virtual_without_transient (var&& self, const var& key) { return virtual_without_persistent (self, key); }

	inline some<Value, never_nil>
	Value::without_persistent (const var& self, const var& key) const { return virtual_without_persistent (self, key); }
	inline some<Value, maybe_nil>
	Value::without_transient (var&& self, const var& key) { return virtual_without_transient (std::move (self), key); }

	inline generic_enumerator Value::virtual_enumerate () const { throw operation_not_supported (); }
	inline generic_enumerator Value::enumerate () const { return virtual_enumerate (); }

	inline var Value::virtual_first () const { return virtual_nth (0); }
	inline var Value::first () const { return virtual_first (); }

	inline var Value::virtual_second () const { return virtual_nth (1); }
	inline var Value::second () const { return virtual_second (); }

	inline var Value::virtual_nth (intptr_t n) const { throw operation_not_supported (); }
	inline var Value::nth (intptr_t n) const { return virtual_nth (n); }

	inline some<Value, never_nil>
	Value::virtual_append_persistent (const var& self, var&& element) const { throw operation_not_supported (); }
	inline some<Value, maybe_nil> Value::virtual_append_transient (var&& self, var&& element) {
		return virtual_append_persistent (self, std::move (element));
	}

	template<typename Arg>
	inline some<Value, never_nil> Value::append_persistent (const var& self, Arg&& arg) const {
		return virtual_append_persistent (self, detail::move_or_copy (
				std::forward<Arg> (arg)));
	}
	template<typename Arg>
	inline some<Value, maybe_nil> Value::append_transient (var&& self, Arg&& arg) {
		return virtual_append_transient (std::move (self), detail::move_or_copy (std::forward<Arg> (arg)));
	}

	template<typename Stream>
	void Value::print_to (Stream& stream) const {
		if constexpr (std::is_base_of_v<var, Stream>)
			return virtual_print_to (stream);
		else {
			if constexpr (!std::is_pointer_v<Stream>) {
				immediate <Boxed<Stream*>> generic_stream {&stream};
				return virtual_print_to (generic_stream);
			}
			else {
				immediate <Boxed<Stream>> generic_stream {stream};
				return virtual_print_to (generic_stream);
			}

		}
	}
}
