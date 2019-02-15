#pragma once

#include <pure/types/any.hpp>

namespace pure {
	struct nil_t : any {
		using domain_t = Nil_t;

		nil_t () { this->init_nil (); };

		template<typename T>
		nil_t (const T& other) {
			assert (Var::tag (other) == Var::Tag::Nil);
			this->init_nil ();
		}


		~nil_t () { this->init_nil (); };
		constexpr Var::Tag tag () const noexcept { return Var::Tag::Nil; }
	};
}