#pragma once

#include <pure/type_class.hpp>
#include <pure/symbolic_sets.hpp>
#include <pure/traits.hpp>
#include <pure/support/endian.hpp>
#include <pure/support/misc.hpp>

namespace pure {

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
	 Base of the value holder system. Every value holder inherits from var, therefore const var& and var&& are
	 very flexible candidates for argument types.
	 Objects of type var can hold arbitrary values, so var adds a completely dynamic type to C++. Booleans, integers,
	 doubles, characters and small strings are allocated in place.
	 */
	struct var : implements<Type_Class::Var> {
		using domain_t = Any_t;
		using object_type = Interface::Value;

		enum Var::Tag m_tag;

		union {
			Interface::Value* ptr;
			int64_t _int;
			char32_t _char;
			double _double;
			char str[8];
		} m_value ;

		constexpr var() : m_tag {Var::Tag::Nil}, m_value {nullptr} {}

		template<typename T>
		var (T&& other) {
			init_var (Var::tag (other), std::forward<T> (other));
		}
		var (const var& other) : var (static_cast<const var&&> (other)) {}

		~var ();

		template<typename T>
		const var& operator= (T&& other) {
			if ((void*) &other == (void*) this) return *this;
			else {
				this->~var ();
				this->init_nil ();
				new (this) var {std::forward<T> (other)};
				return *this;
			}
		}

		const var& operator= (const var& other) { return (*this = static_cast<const var&&> (other)); }

		template<typename T>
		static constexpr bool transient_reset_accepts = false;

		static constexpr bool definitely_mutable_pointer = false;
		static constexpr bool definitely_const_pointer = false;
		static constexpr bool definitely_pointer = false;
		static constexpr bool maybe_nil = true;

		Interface::Value* operator-> () const noexcept {
			return m_value.ptr;
		}

		const Interface::Value& operator* () const noexcept {
			return *(this->operator-> ());
		}
		Interface::Value& operator* () noexcept {
			return *(this->operator-> ());
		}

		Interface::Value* release () noexcept {
			auto tmp = this->operator-> ();
			this->init_nil();
			return tmp;
		}

		enum Var::Tag tag () const noexcept {
			return m_tag;
		}

		intptr_t get_int () const noexcept {
			return static_cast<intptr_t>(m_value._int);
		}

		char32_t get_char () const noexcept { return m_value._char; }
		double get_double () const noexcept { return m_value._double; }
		int64_t get_int64 () const noexcept {return m_value._int; }

		const char* get_cstring () const noexcept {
			return m_value.str;
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
			return value <= INTPTR_MAX && value >= INTPTR_MIN;
		}

	protected:
		Var::Tag read_tag () const noexcept { return m_tag; }
		Interface::Value* read_ptr () const noexcept { return this->operator-> (); }

		template<typename T>
		void init_var (Var::Tag tag, T&& other);

		void init_ptr (Var::Tag tag, Interface::Value* ptr) noexcept {
			m_tag = tag;
			m_value.ptr = ptr;
		}

		void init_tag (Var::Tag tag) noexcept { init_ptr (tag, nullptr); }

		void init_nil () noexcept { m_tag = Var::Tag::Nil; m_value.ptr = nullptr; }

		template<typename int_type>
		void init_int (Var::Tag tag, int_type value) noexcept {
			m_tag = tag;
			m_value._int = value;
		}

		void init_double (double value) noexcept {
			m_tag = Var::Tag::Double;
			m_value._double = value;
		}

		template<intptr_t capacity, typename T>
		void init_small_string (Var::Tag tag, T&& other) {
			auto str = Var::get_cstring (other);
			auto length = Var::get_cstring_length (other);
			if (length < capacity) {
				m_tag = tag;
				std::memcpy (m_value.str, str, length + 1);
				std::memset (m_value.str + length + 1, 0, capacity - (length + 1));
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
#include <pure/object/interface.hpp>

namespace pure {
	inline var::~var () {
		switch (tag ()) {
			case Var::Tag::Unique : delete this->operator-> ();
				return;
			case Var::Tag::Shared : if (detail::dec_ref_count (this->operator* ())) delete this->operator-> ();
				return;
			default : return;
		}
	}

	template<typename T>
	inline void var::init_var (Var::Tag tag, T&& other) {
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
			case Tag::Int64 : init_int (Tag::Int64, Var::get_int64 (other));
				return;
			case Tag::Double : init_double (Var::get_double (other));
				return;
			case Tag::Char : init_int (Tag::Char, Var::get_char (other));
				return;
			case Tag::String : init_small_string<8> (Tag::String, std::forward<T> (other));
				return;
			default : init_ptr (Tag::Shared, Var::clone (std::forward<T> (other)));
				return;
		}
	}

	template<typename... Args>
	inline auto var::operator() (Args&& ... args) const {
		switch (tag ()) {
			case Var_Tag_Pointer : return this->operator-> ()->apply (std::forward<Args> (args)...);
			default : throw operation_not_supported ();
		}
	}

	inline var var::operator- () const {
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

namespace pure {
	namespace Interface {
		var Exception::error_message () { return what (); }
		int Exception::error_code () noexcept { return -1; }
		var Exception::error_type () { return "std::exception"; }
		var Exception::error_data () { return nullptr; }
	}
}
