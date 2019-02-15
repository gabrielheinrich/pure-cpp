#pragma once

#include <pure/types/any.hpp>

namespace pure {
	/**
	 Value holder similar to any, but 128 bit in size, so both double and int64 values don't have to be allocated on
	 the heap.
	 */
	struct var : any {
		union {
			int64_t _int64_t;
			double _double;
		} immediate;

		var () {};
		template<typename T>
		var (T&& other) {
			auto tag = Var::tag (other);
			switch (tag) {
				case Var::Tag::Int : init_int64 (Var::get_int (other));
					return;
				case Var::Tag::Int64 : init_int64 (Var::get_int64 (other));
					return;
				case Var::Tag::Double : init_double (Var::get_double (other));
					return;
				case Var::Tag::String :
					init_small_string<sizeof (*this) - 2> (Var::Tag::String, std::forward<T> (other));
					return;
				default : this->init_any (tag, std::forward<T> (other));
					immediate._int64_t = 0;
					return;
			}
		}

		var (const var& other) : var {static_cast<const var&&>(other)} {};

		var (var&& other) {
			std::memcpy (this, &other, sizeof (var));
			other.init_nil ();
		}

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

	protected:
		void init_int64 (int64_t i) {
			this->init_tag (Var::Tag::Int64);
			immediate._int64_t = i;
		}

		void init_double (double d) {
			this->init_tag (Var::Tag::Double);
			immediate._double = d;
		}
	};
}

namespace pure {
	namespace Interface {
		var Exception::error_message () { return what (); }
		int Exception::error_code () noexcept { return -1; }
		var Exception::error_type () { return "std::exception"; }
		var Exception::error_data () { return nullptr; }
	}
}
