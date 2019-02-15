#pragma once

#include <pure/object/interface.hpp>
#include <pure/traits.hpp>
#include <pure/types/weak.hpp>

namespace pure {
	/**
	 Automatically generated version of an unboxed type into one that inherits from Interface::Value and can
	 therefore be used within a value holder. Works by forwarding the dynamic calls to virtual functions to the
	 statically typed versions on the boxed type.
	 */
	template<typename T_>
	struct Boxed : Interface::Value {
		static_assert (!std::is_base_of_v<any, T_>);
		T_ self;
		static constexpr bool is_pointer = std::is_pointer_v<T_> && !std::is_same_v<T_, FILE*>;

		using T = std::conditional_t<is_pointer, std::remove_pointer_t<T_>, T_>;

		const auto& get () const {
			if constexpr (is_pointer) return *self;
			else return self;
		}

		auto& get () {
			if constexpr (is_pointer) return *self;
			else return self;
		}

		template<typename Arg>
		void reset (Arg&& other) {
			self = std::forward<Arg> (other);
		}

		template<typename... Args>
		Boxed (Args&& ... args) : self (std::forward<Args> (args)...) {}

		using domain_t = pure::domain_t<T>;
		int category_id () const noexcept override { return pure::category_id (get ()); }

		Value* clone () const& override {
			if constexpr (is_pointer) return new Boxed<T> {*self};
			else return new Boxed {self};
		}

		Value* clone ()&& override {
			if constexpr (is_pointer) return new Boxed<T> {*self};
			else return new Boxed {std::move (self)};
		}
		intptr_t clone_bytes_needed () const override { return is_pointer ? sizeof (Boxed<T>) : sizeof (Boxed); }

		Value* clone_placement (void* memory, intptr_t num_bytes) const& override {
			if constexpr (is_pointer) return new (memory) Boxed<T> {*self};
			else return new (memory) Boxed {self};
		}
		Value* clone_placement (void* memory, intptr_t num_bytes)&& override {
			if constexpr (is_pointer) return new (memory) Boxed<T> {*self};
			else return new (memory) Boxed {std::move (self)};
		}

		int64_t get_int64 () const override {
			if constexpr (std::is_convertible_v<T, int64_t>)
				return static_cast<int64_t> (get ());
			else throw operation_not_supported ();
		}

		double get_double () const override {
			if constexpr (std::is_convertible_v<T, double>)
				return static_cast<double> (get ());
			else throw operation_not_supported ();
		}

		bool get_bool () const noexcept override {
			if constexpr (std::is_convertible_v<T, bool>)
				return static_cast<bool> (get ());
			else
				return true;
		}

		const char* cstring () const override {
			if constexpr (Trait_CString<T>::implemented)
				return pure::raw_cstring (get ());
			else throw operation_not_supported ();
		}

		intptr_t cstring_length () const override {
			if constexpr (Trait_CString<T>::implemented)
				return pure::raw_cstring_length (get ());
			else throw operation_not_supported ();
		}

		bool equal (const weak<>& other) const override {
			if constexpr (Trait_Compare<T, weak<>>::implemented)
				return pure::equal (get (), other);
			else throw operation_not_supported ();
		}

		bool equivalent (const weak<>& other) const override {
			if constexpr (detail::equality_comparable<T, weak<>>)
				return get () == other;
			else throw operation_not_supported ();
		}

		int compare (const weak<>& other) const override {
			if constexpr (Trait_Compare<T, weak<>>::comparable)
				return pure::compare (get (), other);
			else throw operation_not_supported ();
		}

		int equivalent_compare (const weak<>& other) const override {
			if constexpr (Trait_Compare<T, weak<>>::comparable)
				return pure::equivalent_compare (get (), other);
			else throw operation_not_supported ();
		}

		int32_t hash () const override {
			if constexpr (Trait_Hash<T>::implemented)
				return pure::hash (get ());
			else throw operation_not_supported ();
		}

		static constexpr bool is_Functional = Trait_Functional<T>::implemented;
		var virtual_apply () const override {
			if constexpr (is_Functional) {
				if constexpr (pure::Applicable<T>) {
					if constexpr (std::is_void_v<decltype (pure::apply (get ()))>)
						return pure::apply (get ()), nullptr;
					else
						return pure::apply (get ());
				}
			}
			throw operation_not_supported ();
		}

		var virtual_apply (const any& a0) const override {
			if constexpr (is_Functional) {
				if constexpr (pure::Applicable<T, const any&>) {
					if constexpr (std::is_void_v<decltype (pure::apply (get (), a0))>)
						return pure::apply (get (), a0), nullptr;
					else
						return pure::apply (get (), a0);
				}
			}
			throw operation_not_supported ();
		}

		var virtual_apply (const any& a0, const any& a1) const override {
			if constexpr (is_Functional) {
				if constexpr (pure::Applicable<T, const any&, const any&>) {
					if constexpr (std::is_void_v<decltype (pure::apply (get (), a0, a1))>)
						return pure::apply (get (), a0, a1), nullptr;
					else
						return pure::apply (get (), a0, a1);
				}
			}
			throw operation_not_supported ();
		}

		var virtual_apply (const any& a0, const any& a1, const any& a2) const override {
			if constexpr (is_Functional) {
				if constexpr (pure::Applicable<T, const any&, const any&, const any&>) {
					if constexpr (std::is_void_v<decltype (pure::apply (get (), a0, a1, a2))>) {
						return pure::apply (get (), a0, a1, a2), nullptr;
					}
					else {
						return pure::apply (get (), a0, a1, a2);
					}
				}
				else {
					throw operation_not_supported ();
				}
			}
			else {
				throw operation_not_supported ();
			}
		}

		template<typename... Args>
		auto apply (Args&& ... args) {
			if constexpr (is_Functional)
				if constexpr (pure::Applicable<T, Args&&...>)
					return pure::apply (get (), std::forward<Args> (args)...);
				else
					throw operation_not_supported ();
			else
				throw operation_not_supported ();
		}

		intptr_t arity () const override {
			if constexpr (is_Functional)
				return pure::arity (get ());
			else
				throw operation_not_supported ();
		}

		bool Variadic () const noexcept override {
			if constexpr (is_Functional)
				return pure::Variadic (get ());
			else return false;
		}

		some<Value, never_nil> virtual_set_persistent (const any&, any&& key, any&& value) const override {
			if constexpr (is_Functional)
				return pure::set (get (), std::move (key), std::move (value));
			else
				throw operation_not_supported ();
		};

		some<Value, maybe_nil> virtual_set_transient (any&&, any&& key, any&& value) override {
			if constexpr (is_Functional)
				return pure::set (std::move (get ()), std::move (key), std::move (value));
			else
				throw operation_not_supported ();
		};

		template<typename... Args>
		auto set_persistent (const any&, Args&& ... args) const {
			if constexpr (is_Functional)
				return pure::set (get (), std::forward<Args> (args)...);
			else {
				throw operation_not_supported ();
				return nullptr;
			}
		}

		template<typename... Args>
		auto set_transient (any&&, Args&& ... args) {
			if constexpr (is_Functional)
				return pure::set (std::move (get ()), std::forward<Args> (args)...);
			else {
				throw operation_not_supported ();
				return nullptr;
			}
		}

		bool Enumerable () const noexcept override {
			return pure::Enumerable (get ());
		}

		static constexpr bool is_Enumerable = Trait_Enumerable<T>::implemented;
		auto enumerate () const {
			if constexpr (is_Enumerable)
				return pure::enumerate (get ());
			else
				throw operation_not_supported ();
		}

		generic_enumerator virtual_enumerate () const override {
			if constexpr (is_Enumerable)
				return pure::enumerate (get ());
			else
				throw operation_not_supported ();
		}

		intptr_t count () const override {
			if constexpr (is_Enumerable)
				return pure::count (get ());
			else
				throw operation_not_supported ();
		}

		bool Empty () const override {
			if constexpr (is_Enumerable)
				return pure::Empty (get ());
			else
				throw operation_not_supported ();
		}

		auto first () const {
			if constexpr (is_Enumerable) return pure::first (get ());
			else throw operation_not_supported ();
		}

		var virtual_first () const override {
			if constexpr (is_Enumerable) return pure::first (get ());
			else throw operation_not_supported ();
		}

		auto second () const {
			if constexpr (is_Enumerable) return pure::second (get ());
			else throw operation_not_supported ();
		}

		var virtual_second () const override {
			if constexpr (is_Enumerable) return pure::second (get ());
			else throw operation_not_supported ();
		}

		auto nth (intptr_t n) const {
			if constexpr (is_Enumerable) return pure::nth (get (), n);
			else throw operation_not_supported ();
		}

		var virtual_nth (intptr_t n) const override {
			if constexpr (is_Enumerable) return pure::nth (get (), n);
			else throw operation_not_supported ();
		}

		some<> virtual_append_persistent (const any&, any&& element) const override {
			if constexpr (is_Enumerable) return pure::append (get (), std::move (element));
			else throw operation_not_supported ();
		}

		maybe<> virtual_append_transient (any&&, any&& element) override {
			if constexpr (is_Enumerable) return pure::append (std::move (get ()), std::move (element));
			else throw operation_not_supported ();
		}

		template<typename Arg>
		auto append_persistent (const any&, Arg&& element) const {
			if constexpr (is_Enumerable) return pure::append (get (), std::forward<Arg> (element));
			else throw operation_not_supported ();
		}

		template<typename Arg>
		auto append_transient (any&&, Arg&& element) {
			if constexpr (is_Enumerable) return pure::append (std::move (get ()), std::forward<Arg> (element));
			else throw operation_not_supported ();
		}

		const char* error_message () const override {
			if constexpr (Trait_Error<T>::implemented)
				return pure::error_message (get ());
			else
				throw operation_not_supported ();
		}

		void virtual_print_to (any& stream) const override {
			return this->print_to (stream);
		}

		template<typename Stream>
		void print_to (Stream& stream) const {
			if constexpr (Trait_Print<T>::implemented)
				IO::print_to (stream, get ());
			else throw operation_not_supported ();
		};

		bool write_raw_bytes (const uint8_t* data, intptr_t num_bytes) override {
			if constexpr (Trait_Stream<T>::implemented)
				return IO::write_raw_bytes (get (), data, num_bytes);
			else throw operation_not_supported ();
		}

		bool write_raw_string (const char* data) override {
			if constexpr (Trait_Stream<T>::implemented)
				return IO::write_raw_string (get (), data);
			else throw operation_not_supported ();
		}

		bool write_raw_byte (uint8_t byte) override {
			if constexpr (Trait_Stream<T>::implemented)
				return IO::write_raw_byte (get (), byte);
			else throw operation_not_supported ();
		}

		bool vprintf (const char* format, va_list& args) override {
			if constexpr (Trait_Stream<T>::implemented)
				return Trait_Stream<T>::vprintf (get (), format, args);
			else throw operation_not_supported ();
		}
	};
}
