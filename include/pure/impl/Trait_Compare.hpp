#pragma once

#include <pure/traits.hpp>
#include <pure/types/var.hpp>
#include <pure/support/utf8.hpp>
#include <pure/support/tuple.hpp>

namespace pure {
	namespace detail {

		template<typename LHS, typename RHS> using simple_compare_type = decltype (LHS{0} + RHS{0});

		template<typename LHS, typename RHS>
		int simple_compare (LHS lhs, RHS rhs) {
			using T = simple_compare_type<LHS, RHS>;
			auto lhs_ = (T) lhs;
			auto rhs_ = (T) rhs;
			return lhs_ < rhs_ ? -1 : lhs_ == rhs_ ? 0 : 1;
		}

		template<typename T>
		int signum (T value) { return (T (0) < value) - (value < T (0)); }

		template<typename LHS, typename RHS>
		int compare_equivalent_scalar (LHS lhs, const RHS& rhs) {
			static_assert (std::is_arithmetic_v<LHS>);
			if constexpr (std::is_arithmetic_v < RHS >) {
				return simple_compare (lhs, rhs);
			}
			else {
				switch (Var::tag (rhs)) {
					case Var::Tag::False : return simple_compare (lhs, false);
					case Var::Tag::True : return simple_compare (lhs, true);
					case Var::Tag::Int : return simple_compare (lhs, Var::get_int (rhs));
					case Var::Tag::Int64 : return simple_compare (lhs, Var::get_int64 (rhs));
					case Var::Tag::Double : return simple_compare (lhs, Var::get_double (rhs));
					case Var::Tag::Char: return simple_compare (lhs, Var::get_char (rhs));
					default: return false;
				}
			}
		};

		template<typename LHS, typename RHS>
		int equivalent_scalar (LHS lhs, const RHS& rhs) {
			static_assert (std::is_arithmetic_v<LHS>);
			if constexpr (std::is_arithmetic_v < RHS >) {
				return lhs == rhs;
			}
			else {
				switch (Var::tag (rhs)) {
					case Var::Tag::False : return (bool)lhs == false;
					case Var::Tag::True : return (bool)lhs == true;
					case Var::Tag::Int : return (intptr_t)lhs == Var::get_int (rhs);
					case Var::Tag::Int64 : return (int64_t)lhs == Var::get_int64 (rhs);
					case Var::Tag::Double : return (double)lhs == Var::get_double (rhs);
					case Var::Tag::Char: return (char32_t)lhs == Var::get_char (rhs);
					default : return false;
				}
			}
		};

		template<typename Eq, typename LHS, typename RHS>
		bool equal_enumerator (const Eq& eq, LHS&& lhs, RHS&& rhs) {
			if (lhs.has_size () && rhs.has_size () && lhs.size () != rhs.size ()) return false;

			while (!lhs.empty ()) {
				if (rhs.empty ()) return false;
				if (!eq (lhs.read (), rhs.read ())) return false;
				lhs.next ();
				rhs.next ();
			}
			return rhs.empty ();
		};

		template<typename Eq, typename LHS, typename RHS>
		bool equal_sequence (const Eq& eq, LHS&& lhs, const RHS& rhs) {
			if constexpr (Trait_Enumerable<RHS>::implemented) {
				if (pure::category_id (rhs) == Any_Vector.id && pure::Enumerable (rhs)) {
					return equal_enumerator (eq, lhs, pure::enumerate (rhs));
				}
			}
			return false;
		};

		template<typename Eq, typename LHS, typename RHS>
		bool equal_map (const Eq& eq, LHS&& lhs, intptr_t lhs_count, const RHS& rhs) {
			if constexpr (Trait_Enumerable<RHS>::implemented) {
				if (pure::category_id (rhs) == Any_Function.id && pure::count (rhs) == lhs_count) {
					for (; !lhs.empty (); lhs.next ()) {
						auto&& element = lhs.read ();
						if (!eq (pure::apply (rhs, pure::first (element)), pure::second (element))) return false;
					}
					return true;
				}
			}
			return false;
		};

		template<typename Cmp, typename LHS, typename RHS>
		int compare_enumerator (const Cmp& cmp, LHS&& lhs, RHS&& rhs) {
			while (!lhs.empty ()) {
				if (rhs.empty ()) return 1;
				if (auto c = cmp (lhs.read (), rhs.read ());
				c) return c;
				lhs.next ();
				rhs.next ();
			}
			return rhs.empty () ? 0 : -1;
		};

		template<typename Cmp, typename LHS, typename RHS>
		int compare_sequence (const Cmp& cmp, LHS&& lhs, const RHS& rhs) {
			if constexpr (Trait_Enumerable<RHS>::implemented) {
				if (pure::category_id (rhs) == Any_Vector.id && pure::Enumerable (rhs)) {
					return compare_enumerator (cmp, lhs, pure::enumerate (rhs));
				}
			}
			throw operation_not_supported ();
		};
	}

	template<typename LHS, typename RHS, typename RHS_Type_Class>
	struct Trait_Compare<LHS, RHS, Type_Class::Nil, RHS_Type_Class> : Trait_Definition {
		static constexpr bool comparable = true;
		static bool equal (const LHS& lhs, const RHS& rhs) { return Var::tag (rhs) == Var::Tag::Nil; }

		static bool equivalent (const LHS& lhs, const RHS& rhs) {
			return equal (lhs, rhs);
		}

		static int compare (const LHS& lhs, const RHS& rhs) {
			return detail::simple_compare (pure::category_id (lhs), pure::category_id (rhs));
		}

		static int equivalent_compare (const LHS& lhs, const RHS& rhs) {
			return compare (lhs, rhs);
		}
	};

	template<typename LHS, typename RHS, typename RHS_Type_Class>
	struct Trait_Compare<LHS, RHS, Type_Class::Bool, RHS_Type_Class> : Trait_Definition {
		static constexpr bool comparable = true;
		static bool equal (const LHS& lhs, const RHS& rhs) {
			return Var::tag (lhs) == Var::tag (rhs);
		}

		static bool equivalent (const LHS& lhs, const RHS& rhs) {
			return detail::equivalent_scalar (lhs, rhs);
		}

		static int compare (const LHS& lhs, const RHS& rhs) {
			return detail::simple_compare (pure::category_id (lhs), pure::category_id (rhs));
		}

		static int equivalent_compare (const LHS& lhs, const RHS& rhs) {
			return detail::compare_equivalent_scalar (lhs, rhs);
		}
	};

	template<typename LHS, typename RHS, typename RHS_Type_Class>
	struct Trait_Compare<LHS, RHS, Type_Class::Int, RHS_Type_Class> : Trait_Definition {
		static constexpr bool comparable = true;
		static bool equal (const LHS& lhs, const RHS& rhs) {
			switch (Var::tag (rhs)) {
				case Var::Tag::Int : return lhs == Var::get_int (rhs);
				case Var::Tag::Int64 : return lhs == Var::get_int64 (rhs);
				default :return false;
			}
		}

		static bool equivalent (const LHS& lhs, const RHS& rhs) {
			return detail::equivalent_scalar (lhs, rhs);
		}

		static int compare (const LHS& lhs, const RHS& rhs) {
			switch (Var::tag (rhs)) {
				case Var::Tag::Int : return detail::simple_compare (lhs, Var::get_int (rhs));
				case Var::Tag::Int64 : return detail::simple_compare (lhs, Var::get_int64 (rhs));
				default :return detail::simple_compare (Int.id, pure::category_id (rhs));
			}
		}

		static int equivalent_compare (const LHS& lhs, const RHS& rhs) {
			return detail::compare_equivalent_scalar (lhs, rhs);
		}
	};

	template<typename LHS, typename RHS, typename RHS_Type_Class>
	struct Trait_Compare<LHS, RHS, Type_Class::Double, RHS_Type_Class> : Trait_Definition {
		static constexpr bool comparable = true;
		static bool equal (const LHS& lhs, const RHS& rhs) {
			switch (Var::tag (rhs)) {
				case Var::Tag::Double : return lhs == Var::get_double (rhs);
				default : return false;
			}
		}

		static bool equivalent (const LHS& lhs, const RHS& rhs) {
			return detail::equivalent_scalar (lhs, rhs);
		}

		static int compare (const LHS& lhs, const RHS& rhs) {
			switch (Var::tag (rhs)) {
				case Var::Tag::Double : return detail::simple_compare (lhs, Var::get_double (rhs));
				default :return detail::simple_compare (Double.id, pure::category_id (rhs));
			}
		}

		static int equivalent_compare (const LHS& lhs, const RHS& rhs) {
			return detail::compare_equivalent_scalar (lhs, rhs);
		}
	};

	template<typename LHS, typename RHS, typename RHS_Type_Class>
	struct Trait_Compare<LHS, RHS, Type_Class::Character, RHS_Type_Class> : Trait_Definition {
		static constexpr bool comparable = true;
		static bool equal (const LHS& lhs, const RHS& rhs) {
			switch (Var::tag (rhs)) {
				case Var::Tag::Char : return lhs == Var::get_char (rhs);
				default : return false;
			}
		}

		static bool equivalent (const LHS& lhs, const RHS& rhs) {
			return detail::equivalent_scalar (lhs, rhs);
		}

		static int compare (const LHS& lhs, const RHS& rhs) {
			switch (Var::tag (rhs)) {
				case Var::Tag::Char : return detail::simple_compare (lhs, Var::get_char (rhs));
				default :return detail::simple_compare (Character.id, pure::category_id (rhs));
			}
		}

		static int equivalent_compare (const LHS& lhs, const RHS& rhs) {
			return detail::compare_equivalent_scalar (lhs, rhs);
		}
	};

	template<typename LHS, typename RHS, typename RHS_Type_Class>
	struct Trait_Compare<LHS, RHS, Type_Class::CString, RHS_Type_Class> : Trait_Definition {
		static constexpr bool comparable = true;
		static bool equal (const LHS& lhs, const RHS& rhs) {
			if constexpr (Trait_CString<RHS>::implemented) {
				if (pure::category_id (lhs) == String.id) {
					return std::strcmp (lhs, pure::raw_cstring (rhs)) == 0;
				}
				return false;
			}
			return false;
		}

		static bool equivalent (const LHS& lhs, const RHS& rhs) {
			return equal (lhs, rhs);
		}

		static int compare (const LHS& lhs, const RHS& rhs) {
			auto rhs_category = pure::category_id (rhs);
			if constexpr (Trait_CString<RHS>::implemented) {
				if (rhs_category == String.id)
					return detail::signum (utf8::string_compare (lhs, pure::raw_cstring (rhs)));
			}
			return detail::simple_compare (String.id, rhs_category);
		}

		static int equivalent_compare (const LHS& lhs, const RHS& rhs) {
			return compare (lhs, rhs);
		}
	};

	template<typename LHS, typename RHS, typename RHS_Type_Class>
	struct Trait_Compare<LHS, RHS, Type_Class::Identifier, RHS_Type_Class> : Trait_Definition {
		static constexpr bool comparable = true;
		static bool equal (const LHS& lhs, const RHS& rhs) {
			if constexpr (Trait_CString<RHS>::implemented) {
				if (pure::category_id (lhs) == String.id) {
					if (lhs.length != pure::raw_cstring_length (rhs)) return false;
					return std::memcmp (lhs.string, pure::raw_cstring (rhs), lhs.length) == 0;
				}
				return false;
			}
			return false;
		}

		static bool equivalent (const LHS& lhs, const RHS& rhs) { return equal (lhs, rhs); }

		static int compare (const LHS& lhs, const RHS& rhs) {
			auto rhs_category = pure::category_id (rhs);
			if constexpr (Trait_CString<RHS>::implemented) {
				if (rhs_category == String.id)
					return detail::signum (utf8::string_compare (lhs.string, pure::raw_cstring (rhs)));
			}
			return detail::simple_compare (String.id, rhs_category);
		}

		static int equivalent_compare (const LHS& lhs, const RHS& rhs) { return compare (lhs, rhs); }
	};

	template<typename LHS, typename RHS>
	struct Trait_Compare<LHS, RHS, Type_Class::Var, Type_Class::Var> : Trait_Definition {
		static constexpr bool comparable = true;
		static bool equal (const LHS& lhs, const RHS& rhs) {
			switch (lhs.tag ()) {
				case Var::Tag::Nil : return pure::equal (nullptr, rhs);
				case Var::Tag::False : return pure::equal (false, rhs);
				case Var::Tag::True : return pure::equal (true, rhs);
				case Var::Tag::Int : return pure::equal (lhs.get_int (), rhs);
				case Var::Tag::Int64 : return pure::equal (lhs.get_int64 (), rhs);
				case Var::Tag::Double : return pure::equal (lhs.get_double (), rhs);
				case Var::Tag::Char : return pure::equal (lhs.get_char (), rhs);
				case Var::Tag::String : return pure::equal (lhs.get_cstring (), rhs);
				case Var_Tag_Pointer : {
					switch (rhs.tag ()) {
						case Var::Tag::String : return pure::equal (rhs.get_cstring (), lhs);
						case Var_Tag_Pointer : return lhs->equal (rhs);
						default : return false;
					}
				}
				default : assert (0);
					return false;
			}
		}

		static bool equivalent (const LHS& lhs, const RHS& rhs) {
			switch (lhs.tag ()) {
				case Var::Tag::Nil : return pure::equivalent (nullptr, rhs);
				case Var::Tag::False : return pure::equivalent (false, rhs);
				case Var::Tag::True : return pure::equivalent (true, rhs);
				case Var::Tag::Int : return pure::equivalent (lhs.get_int (), rhs);
				case Var::Tag::Int64 : return pure::equivalent (lhs.get_int64 (), rhs);
				case Var::Tag::Double : return pure::equivalent (lhs.get_double (), rhs);
				case Var::Tag::Char : return pure::equivalent (lhs.get_char (), rhs);
				case Var::Tag::String : return pure::equivalent (lhs.get_cstring (), rhs);
				case Var_Tag_Pointer : {
					switch (rhs.tag ()) {
						case Var::Tag::String : return pure::equivalent (rhs.get_cstring (), lhs);
						case Var_Tag_Pointer : return lhs->equivalent (rhs);
						default : return false;
					}
				}
				default : assert (0);
					return false;
			}
		}

		static int compare (const LHS& lhs, const RHS& rhs) {
			switch (lhs.tag ()) {
				case Var::Tag::Nil : return pure::compare (nullptr, rhs);
				case Var::Tag::False : return pure::compare (false, rhs);
				case Var::Tag::True : return pure::compare (true, rhs);
				case Var::Tag::Int : return pure::compare (lhs.get_int (), rhs);
				case Var::Tag::Int64 : return pure::compare (lhs.get_int64 (), rhs);
				case Var::Tag::Double : return pure::compare (lhs.get_double (), rhs);
				case Var::Tag::Char : return pure::compare (lhs.get_char (), rhs);
				case Var::Tag::String : return pure::compare (lhs.get_cstring (), rhs);
				case Var_Tag_Pointer : {
					switch (rhs.tag ()) {
						case Var::Tag::Nil : return -pure::compare (nullptr, lhs);
						case Var::Tag::False : return -pure::compare (false, lhs);
						case Var::Tag::True : return -pure::compare (true, lhs);
						case Var::Tag::Int : return -pure::compare (rhs.get_int (), lhs);
						case Var::Tag::Int64 : return -pure::compare (rhs.get_int64 (), lhs);
						case Var::Tag::Double : return -pure::compare (rhs.get_double (), lhs);
						case Var::Tag::Char : return -pure::compare (rhs.get_char (), lhs);
						case Var::Tag::String : return -pure::compare (rhs.get_cstring (), lhs);
						case Var_Tag_Pointer : return lhs->compare (rhs);
						default : assert (0);
							return -1;
					}
				}
				default : assert (0);
					return -1;
			}
		}

		static int equivalent_compare (const LHS& lhs, const RHS& rhs) {
			switch (lhs.tag ()) {
				case Var::Tag::Nil : return pure::equivalent_compare (nullptr, rhs);
				case Var::Tag::False : return pure::equivalent_compare (false, rhs);
				case Var::Tag::True : return pure::equivalent_compare (true, rhs);
				case Var::Tag::Int : return pure::equivalent_compare (lhs.get_int (), rhs);
				case Var::Tag::Int64 : return pure::equivalent_compare (lhs.get_int64 (), rhs);
				case Var::Tag::Double : return pure::equivalent_compare (lhs.get_double (), rhs);
				case Var::Tag::Char : return pure::equivalent_compare (lhs.get_char (), rhs);
				case Var::Tag::String : return pure::equivalent_compare (lhs.get_cstring (), rhs);
				case Var_Tag_Pointer : {
					switch (rhs.tag ()) {
						case Var::Tag::Nil : return -pure::equivalent_compare (nullptr, lhs);
						case Var::Tag::False : return -pure::equivalent_compare (false, lhs);
						case Var::Tag::True : return -pure::equivalent_compare (true, lhs);
						case Var::Tag::Int : return -pure::equivalent_compare (rhs.get_int (), lhs);
						case Var::Tag::Int64 : return -pure::equivalent_compare (rhs.get_int64 (), lhs);
						case Var::Tag::Double : return -pure::equivalent_compare (rhs.get_double (), lhs);
						case Var::Tag::Char : return -pure::equivalent_compare (rhs.get_char (), lhs);
						case Var::Tag::String : return -pure::equivalent_compare (rhs.get_cstring (), lhs);
						case Var_Tag_Pointer : return lhs->equivalent_compare (rhs);
						default : assert (0);
							return -1;
					}
				}
				default : assert (0);
					return -1;
			}
		}
	};

	template<typename LHS, typename RHS, typename... Elements, typename RHS_Type_Class>
	struct Trait_Compare<LHS, RHS, Type_Class::Tuple < Elements...>, RHS_Type_Class> : Trait_Definition {
	static constexpr bool comparable = true;

	static bool equal (const LHS& lhs, const RHS& rhs) {
		if constexpr (Trait_Enumerable<RHS>::implemented) {
			if (pure::category_id (rhs) == Any_Vector.id) {
				auto rhs_enum = pure::enumerate (rhs);
				if (rhs_enum.has_size () && rhs_enum.size () != sizeof... (Elements)) return false;
				return detail::tuple::equal_enumerator<0, sizeof... (Elements)> (lhs, rhs_enum);
			}
		}
		return false;
	}

	static bool equivalent (const LHS& lhs, const RHS& rhs) {
		if constexpr (Trait_Enumerable<RHS>::implemented) {
			if (pure::category_id (rhs) == Any_Vector.id) {
				auto rhs_enum = pure::enumerate (rhs);
				if (rhs_enum.has_size () && rhs_enum.size () != sizeof... (Elements)) return false;
				return detail::tuple::equivalent_enumerator<0, sizeof... (Elements)> (lhs, rhs_enum);
			}
		}
		return false;
	}

	static int compare (const LHS& lhs, const RHS& rhs) {
		auto rhs_category = pure::category_id (rhs);
		if (Any_Vector.id == rhs_category) {
			if constexpr (Trait_Enumerable<RHS>::implemented) {
				auto rhs_enum = pure::enumerate (rhs);
				return detail::tuple::compare_enumerator<0, sizeof... (Elements)> (lhs, rhs_enum);
			}
			else throw operation_not_supported ();
		}
		else return detail::simple_compare (Any_Vector.id, rhs_category);
	}
	static int equivalent_compare (const LHS& lhs, const RHS& rhs) {
		auto rhs_category = pure::category_id (rhs);
		if (Any_Vector.id == rhs_category) {
			if constexpr (Trait_Enumerable<RHS>::implemented) {
				auto rhs_enum = pure::enumerate (rhs);
				return detail::tuple::equivalent_compare_enumerator<0, sizeof... (Elements)> (lhs, rhs_enum);
			}
			else throw operation_not_supported ();
		}
		else return detail::simple_compare (Any_Vector.id, rhs_category);
	}
};
//TODO : change so that equal (X, Tuple) doesn't use enumerator

template<typename LHS, typename RHS, typename... LHS_Elements, typename... RHS_Elements>
struct Trait_Compare<LHS, RHS,
		Type_Class::Tuple < LHS_Elements...>, Type_Class::Tuple<RHS_Elements...>> : Trait_Definition {
static constexpr bool comparable = true;

static bool equal (const LHS& lhs, const RHS& rhs) {
	return detail::tuple::do_equal (lhs, rhs, pure::equal);
}
static bool equivalent (const LHS& lhs, const RHS& rhs) {
	return detail::tuple::do_equal (lhs, rhs, pure::equivalent);
}

static int compare (const LHS& lhs, const RHS& rhs) {
	return detail::tuple::do_compare<0, sizeof... (LHS_Elements), sizeof... (RHS_Elements)> (lhs, rhs, pure::compare);
}
static int equivalent_compare (const LHS& lhs, const RHS& rhs) {
	auto result = detail::tuple::do_compare<0, sizeof... (LHS_Elements), sizeof... (RHS_Elements)> (lhs, rhs,
																									pure::equivalent_compare);
	return result;
}
};

template<typename LHS, typename RHS, typename Capabilities, typename RHS_Type_Class>
struct Trait_Compare<LHS, RHS, Type_Class::Iterable < Capabilities>, RHS_Type_Class> : Trait_Definition {
static constexpr bool comparable = true;
static bool equal (const LHS& lhs, const RHS& rhs) {
	if (pure::category_id (rhs) == Any_Vector.id) {
		if constexpr (Trait_Enumerable<RHS>::implemented)
			return detail::equal_enumerator (pure::equal, enumerate (lhs), enumerate (rhs));
		else throw operation_not_supported ();
	}
	else return false;
}

static bool equivalent (const LHS& lhs, const RHS& rhs) {
	if (pure::category_id (rhs) == Any_Vector.id) {
		if constexpr (Trait_Enumerable<RHS>::implemented)
			return detail::equal_enumerator (pure::equivalent, enumerate (lhs), enumerate (rhs));
		else throw operation_not_supported ();
	}
	else return false;
}

static int compare (const LHS& lhs, const RHS& rhs) {
	auto rhs_category = pure::category_id (rhs);
	if (rhs_category == Any_Vector.id) {
		if constexpr (Trait_Enumerable<RHS>::implemented)
			return detail::compare_enumerator (pure::compare, enumerate (lhs), enumerate (rhs));
		else throw operation_not_supported ();
	}
	else return detail::simple_compare (Any_Vector.id, rhs_category);
}

static bool equivalent_compare (const LHS& lhs, const RHS& rhs) {
	auto rhs_category = pure::category_id (rhs);
	if (rhs_category == Any_Vector.id) {
		if constexpr (Trait_Enumerable<RHS>::implemented)
			return detail::compare_enumerator (pure::equivalent_compare, enumerate (lhs), enumerate (rhs));
		else throw operation_not_supported ();
	}
	else return detail::simple_compare (Any_Vector.id, rhs_category);
}
};

template<typename LHS, typename RHS, typename RHS_Type_Class>
struct Trait_Compare<LHS, RHS, Type_Class::Sequence, RHS_Type_Class> : Trait_Definition {
	static constexpr bool comparable = true;
	static bool equal (const LHS& lhs, const RHS& rhs) {
		if (pure::category_id (rhs) == Any_Vector.id) {
			if constexpr (Trait_Enumerable<RHS>::implemented)
				return detail::equal_enumerator (pure::equal, enumerate (lhs), enumerate (rhs));
			else throw operation_not_supported ();
		}
		else return false;
	}

	static bool equivalent (const LHS& lhs, const RHS& rhs) {
		if (pure::category_id (rhs) == Any_Vector.id) {
			if constexpr (Trait_Enumerable<RHS>::implemented)
				return detail::equal_enumerator (pure::equivalent, enumerate (lhs), enumerate (rhs));
			else throw operation_not_supported ();
		}
		else return false;
	}

	static int compare (const LHS& lhs, const RHS& rhs) {
		auto rhs_category = pure::category_id (rhs);
		if (rhs_category == Any_Vector.id) {
			if constexpr (Trait_Enumerable<RHS>::implemented)
				return detail::compare_enumerator (pure::compare, enumerate (lhs), enumerate (rhs));
			else throw operation_not_supported ();
		}
		else return detail::simple_compare (Any_Vector.id, rhs_category);
	}

	static bool equivalent_compare (const LHS& lhs, const RHS& rhs) {
		auto rhs_category = pure::category_id (rhs);
		if (rhs_category == Any_Vector.id) {
			if constexpr (Trait_Enumerable<RHS>::implemented)
				return detail::compare_enumerator (pure::equivalent_compare, enumerate (lhs), enumerate (rhs));
			else throw operation_not_supported ();
		}
		else return detail::simple_compare (Any_Vector.id, rhs_category);
	}
};

}

#include <pure/support/record.hpp>

namespace pure {
template<typename LHS, typename RHS, typename RHS_Type_Class>
struct Trait_Compare<LHS, RHS, Type_Class::Record, RHS_Type_Class> : Trait_Definition {
	static constexpr bool comparable = false;

	static bool equal (const LHS& lhs, const RHS& rhs) {
		if (pure::category_id (rhs) == Any_Function.id) {
			if constexpr (Trait_Enumerable<RHS>::implemented) {
				if (LHS::count != pure::count (rhs)) return false;
				return detail::record_equal_function (pure::equal, lhs, rhs);
			}
			else throw operation_not_supported ();
		}
		return false;
	}

	static bool equivalent (const LHS& lhs, const RHS& rhs) {
		if (pure::category_id (rhs) == Any_Function.id) {
			if constexpr (Trait_Enumerable<RHS>::implemented) {
				if (LHS::count != pure::count (rhs)) return false;
				return detail::record_equal_function (pure::equivalent, lhs, rhs);
			}
			else throw operation_not_supported ();
		}
		return false;
	}
};

template<typename LHS, typename RHS>
struct Trait_Compare<LHS, RHS, Type_Class::Record, Type_Class::Record> : Trait_Definition {
	static constexpr bool comparable = false;

	static bool equal (const LHS& lhs, const RHS& rhs) {
		if (LHS::count != RHS::count) return false;
		return detail::record_equal_record (pure::equal, lhs, rhs);
	}

	static bool equivalent (const LHS& lhs, const RHS& rhs) {
		if (LHS::count != RHS::count) return false;
		return detail::record_equal_record (pure::equivalent, lhs, rhs);
	}
};

}