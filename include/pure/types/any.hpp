#pragma once

#include <pure/type_class.hpp>
#include <pure/symbolic_sets.hpp>
#include <pure/traits.hpp>
#include <pure/support/endian.hpp>
#include <pure/support/misc.hpp>

namespace pure {
	struct var;

	namespace Interface { struct Value; }

	namespace detail {
		static intptr_t logical_shift_right (intptr_t x, intptr_t shift) {
			if constexpr (intptr_t (-1) >> 1 == -1)
				return x >> shift;
			else
				return x < 0
					   ? (uintptr_t (x) >> shift) | (~uintptr_t {0} << (sizeof (intptr_t) * 8 - shift))
					   : uintptr_t (x) >> shift;
		}
	}

	/**
	 Base of the value holder system. Every value holder inherits from any, therefor const any& and any&& are
	 very flexible candidates for argument types.
	 Objects of type any can hold arbitrary values, so any adds a completely dynamic type to C++. Booleans, integers,
	 characters and small strings are allocated in place. any is pointer sized.
	 */
	struct any : implements<Type_Class::Var> {
		using domain_t = Any_t;
		using object_type = Interface::Value;

		uintptr_t value;

		constexpr any () : value {0} {}

		template<typename T>
		any (T&& other) {
			init_any (Var::tag (other), std::forward<T> (other));
		}
		any (const any& other) : any (static_cast<const any&&> (other)) {}

		~any ();

		template<typename T>
		const any& operator= (T&& other) {
			if ((void*) &other == (void*) this) return *this;
			else {
				this->~any ();
				this->init_nil ();
				new (this) any {std::forward<T> (other)};
				return *this;
			}
		}

		const any& operator= (const any& other) { return (*this = static_cast<const any&&> (other)); }

		template<typename T>
		static constexpr bool transient_reset_accepts = false;

		static constexpr bool definitely_mutable_pointer = false;
		static constexpr bool definitely_const_pointer = false;
		static constexpr bool definitely_pointer = false;
		static constexpr bool maybe_nil = true;

		Interface::Value* operator-> () const noexcept {
			if constexpr (detail::endian::native == detail::endian::big)
				return reinterpret_cast<Interface::Value*>(value << 4);
			else
				return reinterpret_cast<Interface::Value*>(value & (~uintptr_t (0xF)));
		}
		const Interface::Value& operator* () const noexcept {
			return *(this->operator-> ());
		}
		Interface::Value& operator* () noexcept {
			return *(this->operator-> ());
		}

		Interface::Value* release () noexcept {
			auto tmp = this->operator-> ();
			value = 0;
			return tmp;
		}

		enum Var::Tag tag () const noexcept {
			if constexpr (detail::endian::native == detail::endian::big) {
				#ifdef PURE_64BIT
					return static_cast<Var::Tag>(value >> 60);
				#else
					return static_cast<Var::Tag>(value >> 28);
				#endif
			}
			else
				return static_cast<Var::Tag>(value & 0xF);
		}

		intptr_t get_int () const noexcept {
			if constexpr (detail::endian::native == detail::endian::big)
				return detail::logical_shift_right (value << 4, 4);
			else
				return detail::logical_shift_right (value, 4);
		}

		char32_t get_char () const noexcept { return get_int (); }
		double get_double () const noexcept;
		int64_t get_int64 () const noexcept;

		const char* get_cstring () const noexcept {
			return reinterpret_cast<const char*>(&value) + 1;
		}

		template<typename... Args>
		auto operator() (Args&& ... args) const;

		template<typename To, typename = std::enable_if_t<Trait_From_Var<std::decay_t<To>>::implemented>>
		operator To () const& {
			return Trait_From_Var<To>::template from<std::decay_t<decltype (*this)>>::convert (*this);
		};

		template<typename To, typename = std::enable_if_t<Trait_From_Var<std::decay_t<To>>::implemented>>
		operator To ()&& {
			return Trait_From_Var<To>::template from<std::decay_t<decltype (*this)>>::convert (std::move (*this));
		};

		template<typename To, typename = std::enable_if_t<Trait_From_Var<std::decay_t<To>>::to_ref_implemented>>
		operator const To& () const& {
			return Trait_From_Var<To>::template from<std::decay_t<decltype (*this)>>::convert (*this);
		};

		var operator- () const;

		static bool int_in_range (int64_t value) {
			#ifdef PURE_64BIT
				static constexpr int64_t Max = (intptr_t (1) << 59) - 1;
				static constexpr int64_t Min = -(intptr_t (1) << 59);
			#else
				static constexpr int64_t Max = (intptr_t (1) << 27) - 1;
				static constexpr int64_t Min = -(intptr_t (1) << 27);
			#endif
				return value <= Max && value >= Min;
		}

	protected:
		Var::Tag read_tag () const noexcept { return tag (); }
		Interface::Value* read_ptr () const noexcept { return this->operator-> (); }

		template<typename T>
		void init_any (Var::Tag tag, T&& other);

		void init_ptr (Var::Tag tag, Interface::Value* ptr) noexcept {
			uintptr_t uptr = reinterpret_cast<uintptr_t> (ptr);
			assert((uptr & (uintptr_t)0xF) == 0);
			uintptr_t utag = static_cast<uintptr_t> (tag);
			if constexpr (detail::endian::native == detail::endian::big) {
				#ifdef PURE_64BIT
					this->value = (utag << 60) | (uptr >> 4);
				#else
					this->value = (utag << 28) | (uptr >> 4);
				#endif
			}
			else
				this->value = utag | uptr;
		}

		void init_tag (Var::Tag tag) noexcept { init_ptr (tag, nullptr); }

		void init_nil () noexcept { this->value = 0; }

		template<typename int_type>
		void init_int (Var::Tag tag, int_type value) noexcept {
			assert (int_in_range (value));
			uintptr_t uvalue = static_cast<uintptr_t> (value);
			uintptr_t utag = static_cast<uintptr_t> (tag);
			if constexpr (detail::endian::native == detail::endian::big) {
				#ifdef PURE_64BIT
					this->value = (utag << 60) | ((uvalue << 4) >> 4);
				#else
					this->value = (utag << 28) | ((uvalue << 4) >> 4);
				#endif
			}
			else
				this->value = utag | (uvalue << 4);
		}

		template<intptr_t capacity, typename T>
		void init_small_string (Var::Tag tag, T&& other) {
			auto str = Var::get_cstring (other);
			auto length = Var::get_cstring_length (other);
			if (length < capacity) {
				reinterpret_cast<uint8_t*>(&value)[0] = static_cast<uint8_t> (tag);
				std::memcpy (reinterpret_cast<char*>(&value) + 1, str, length + 1);
				std::memset (reinterpret_cast<char*>(&value) + length + 2, 0, capacity - length);
			}
			else {
				if constexpr (Var::has_interned<T>)
					init_ptr (Var::Tag::Interned, Var::obj (other));
				else
					init_ptr (Var::Tag::Shared, Var::clone (std::forward<T> (other)));
			}
		};
	};
}

// inline implementations
#include <pure/types/var.hpp>
#include <pure/object/interface.hpp>

namespace pure {
	inline any::~any () {
		switch (tag ()) {
			case Var::Tag::Unique : delete this->operator-> ();
				return;
			case Var::Tag::Shared : if (detail::dec_ref_count (this->operator* ())) delete this->operator-> ();
				return;
			default : return;
		}
	}

	template<typename T>
	inline void any::init_any (Var::Tag tag, T&& other) {
		using namespace Var;
		switch (tag) {
			case Tag::Nil : init_nil ();
				return;
			case Tag::Unique : {
				if constexpr (detail::is_moveable<T&&>)
					init_ptr (Tag::Unique, Var::release (other));
				else
					init_ptr (Tag::Shared, Var::clone (std::forward<T> (other)));
			}
				return;
			case Var::Tag::Shared : {
				if constexpr (detail::is_moveable<T&&>)
					init_ptr (Tag::Shared, Var::release (other));
				else
					init_ptr (Tag::Shared, detail::inc_ref_count_and_get (*Var::obj (other)));
			}
				return;
			case Tag::Interned : init_ptr (Var::Tag::Interned, Var::obj (other));
				return;
			case Tag::True : init_tag (Tag::True);
				return;
			case Tag::False : init_tag (Tag::False);
				return;
			case Tag::Int : init_int (Tag::Int, Var::get_int (other));
				return;
			case Tag::Char : init_int (Tag::Char, Var::get_char (other));
				return;
			case Tag::String : init_small_string<sizeof (*this) - 2> (Tag::String, std::forward<T> (other));
				return;
			default : init_ptr (Tag::Shared, Var::clone (std::forward<T> (other)));
				return;
		}
	}

	template<typename... Args>
	inline auto any::operator() (Args&& ... args) const {
		switch (tag ()) {
			case Var_Tag_Pointer : return this->operator-> ()->apply (std::forward<Args> (args)...);
			default : throw operation_not_supported ();
		}
	}

	inline double any::get_double () const noexcept {
		return static_cast<const var*>(this)->immediate._double;
	}
	inline int64_t any::get_int64 () const noexcept {
		return static_cast<const var*>(this)->immediate._int64_t;
	}

	inline var any::operator- () const {
		using namespace Var;
		switch (tag ()) {
			case Tag::Nil :
			case Tag::False : return 0;
			case Tag::True : return -1;
			case Tag::Int : return -get_int ();
			case Tag::Int64 : return -get_int64 ();
			case Tag::Double : return -get_double ();
			case Tag::Char : return -get_int ();
			case Var_Tag_Pointer : {
				switch (this->operator-> ()->category_id ()) {
					case Int.id :
					case True.id :
					case False.id :
					case Character.id : return -this->operator-> ()->get_int64 ();
					case Double.id : return -this->operator-> ()->get_double ();
				}
			}
			default : throw operation_not_supported ();
		}
	}
}
