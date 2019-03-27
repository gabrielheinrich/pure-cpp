#pragma once

#include <pure/traits.hpp>

namespace pure {
	template<typename LHS, typename RHS, typename RHS_Type_Class>
	struct Trait_Arithmetic<LHS, RHS, Type_Class::Var, RHS_Type_Class> : Trait_Definition {
		static var add (const LHS& lhs, const RHS& rhs) {
			switch (lhs.tag ()) {
				case Var::Tag::False : return false + rhs;
				case Var::Tag::True : return true + rhs;
				case Var::Tag::Int : return lhs.get_int () + rhs;
				case Var::Tag::Int64 : return lhs.get_int64 () + rhs;
				case Var::Tag::Double : return lhs.get_double () + rhs;
				case Var::Tag::Char : return lhs.get_char () + rhs;
				default : throw operation_not_supported ();
			}
		}

		static var sub (const LHS& lhs, const RHS& rhs) {
			switch (lhs.tag ()) {
				case Var::Tag::False : return false - rhs;
				case Var::Tag::True : return true - rhs;
				case Var::Tag::Int : return lhs.get_int () - rhs;
				case Var::Tag::Int64 : return lhs.get_int64 () - rhs;
				case Var::Tag::Double : return lhs.get_double () - rhs;
				case Var::Tag::Char : return lhs.get_char () - rhs;
				default : throw operation_not_supported ();
			}
		}

		static var mul (const LHS& lhs, const RHS& rhs) {
			switch (lhs.tag ()) {
				case Var::Tag::False : return false * rhs;
				case Var::Tag::True : return true * rhs;
				case Var::Tag::Int : return lhs.get_int () * rhs;
				case Var::Tag::Int64 : return lhs.get_int64 () * rhs;
				case Var::Tag::Double : return lhs.get_double () * rhs;
				case Var::Tag::Char : return lhs.get_char () * rhs;
				default : throw operation_not_supported ();
			}
		}

		static var div (const LHS& lhs, const RHS& rhs) {
			switch (lhs.tag ()) {
				case Var::Tag::False : return false / rhs;
				case Var::Tag::True : return true / rhs;
				case Var::Tag::Int : return lhs.get_int () / rhs;
				case Var::Tag::Int64 : return lhs.get_int64 () / rhs;
				case Var::Tag::Double : return lhs.get_double () / rhs;
				case Var::Tag::Char : return lhs.get_char () / rhs;
				default : throw operation_not_supported ();
			}
		}

		static var mod (const LHS& lhs, const RHS& rhs) {
			switch (lhs.tag ()) {
				case Var::Tag::False : return false % rhs;
				case Var::Tag::True : return true % rhs;
				case Var::Tag::Int : return lhs.get_int () % rhs;
				case Var::Tag::Int64 : return lhs.get_int64 () % rhs;
				case Var::Tag::Char : return lhs.get_char () % rhs;
				default : throw operation_not_supported ();
			}
		}
	};

	template<typename LHS, typename RHS, typename LHS_Type_Class>
	struct Trait_Arithmetic<LHS, RHS, LHS_Type_Class, Type_Class::Var> : Trait_Definition {
		static var add (const LHS& lhs, const RHS& rhs) { return rhs + lhs; }

		static var sub (const LHS& lhs, const RHS& rhs) {
			switch (rhs.tag ()) {
				case Var::Tag::False : return lhs - false;
				case Var::Tag::True : return lhs - true;
				case Var::Tag::Int : return lhs - rhs.get_int ();
				case Var::Tag::Int64 : return lhs - rhs.get_int64 ();
				case Var::Tag::Double : return lhs - rhs.get_double ();
				case Var::Tag::Char : return lhs - rhs.get_char ();
				default : throw operation_not_supported ();
			}
		}

		static var mul (const LHS& lhs, const RHS& rhs) { return rhs * lhs; }

		static var div (const LHS& lhs, const RHS& rhs) {
			switch (rhs.tag ()) {
				case Var::Tag::False : return lhs / false;
				case Var::Tag::True : return lhs / true;
				case Var::Tag::Int : return lhs / rhs.get_int ();
				case Var::Tag::Int64 : return lhs / rhs.get_int64 ();
				case Var::Tag::Double : return lhs / rhs.get_double ();
				case Var::Tag::Char : return lhs / rhs.get_char ();
				default : throw operation_not_supported ();
			}
		}

		static var mod (const LHS& lhs, const RHS& rhs) {
			switch (rhs.tag ()) {
				case Var::Tag::False : return lhs % false;
				case Var::Tag::True : return lhs % true;
				case Var::Tag::Int : return lhs % rhs.get_int ();
				case Var::Tag::Int64 : return lhs % rhs.get_int64 ();
				case Var::Tag::Char : return lhs % rhs.get_char ();
				default : throw operation_not_supported ();
			}
		}
	};

	template<typename LHS, typename RHS>
	struct Trait_Arithmetic<LHS, RHS, Type_Class::Var, Type_Class::Var>
			: Trait_Arithmetic<LHS, RHS, Type_Class::Var, void> {
	};
}