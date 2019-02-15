#include <pure/core.hpp>

#include "catch/catch.hpp"
#include <tuple>
#include <vector>
#include <unordered_map>

using namespace pure;

TEST_CASE ("domain") {
	SECTION ("Builtin Types") {
		REQUIRE (domain<std::nullptr_t> (nullptr));
		REQUIRE (domain<std::nullptr_t> (0) == false);
		REQUIRE (domain<bool> (true));
		REQUIRE (domain<bool> (false));
		REQUIRE (domain<bool> (0) == false);
		REQUIRE (domain<bool> (nullptr) == false);
		REQUIRE (domain<int> (0));
		REQUIRE (domain<signed char> (0));
		REQUIRE (domain<signed char> ('a') == false);
		REQUIRE (domain<int> (0.0) == false);
		REQUIRE (domain<double> (0.0));
		REQUIRE (domain<double> (0) == false);
		REQUIRE (domain<char> ('a'));
		REQUIRE (domain<char32_t> ('a'));
		REQUIRE (domain<const char*> ("Hello World"));
		REQUIRE (domain<const char*> (any ("Hello World")));
		REQUIRE (domain<FILE*> (stdin));
		REQUIRE (domain<FILE*> (nullptr) == false);
	} SECTION ("Value Holder Types") {
		REQUIRE (domain<any> (nullptr));
		REQUIRE (domain<any> (true));
		REQUIRE (domain<any> (0));
		REQUIRE (domain<any> (0.0));
		REQUIRE (domain<any> ('a'));
		REQUIRE (domain<any> ("Hello World"));
		REQUIRE (domain<some<Basic::String>> ("Hello World"));
		REQUIRE_FALSE (domain<some<Basic::String>> (0));
		REQUIRE_FALSE (domain<some<Basic::String>> (nullptr));
		REQUIRE (domain<maybe<Basic::String>> (nullptr));
		REQUIRE (domain<some<Boxed<int>>> (0));
		REQUIRE_FALSE (domain<some<Boxed<int>>> (0.0));
	}
}

TEST_CASE ("definitely_disjunct") {
	SECTION ("Categories") {
		REQUIRE (definitely_disjunct<Nil, Int>);
		REQUIRE_FALSE (definitely_disjunct<Int, Int>);
	} SECTION ("Any") {
		REQUIRE_FALSE (definitely_disjunct<Any, Int>);
		REQUIRE_FALSE (definitely_disjunct<Any, Any>);
		REQUIRE (definitely_disjunct<Any, None>);
		REQUIRE (definitely_disjunct<None, None>);
	} SECTION ("Union") {
		REQUIRE_FALSE (definitely_disjunct<Nil, static_instance<Union_t<Int_t, Nil_t>>::instance>);
		REQUIRE (definitely_disjunct<Nil, static_instance<Union_t<Int_t, Double_t>>::instance>);
		REQUIRE_FALSE (definitely_disjunct_t<Union_t<Nil_t, Int_t>, Union_t<Int_t, Double_t>>::eval());
		REQUIRE (definitely_disjunct_t<Union_t<Nil_t, Int_t>, Union_t<String_t, Double_t>>::eval());
	} SECTION ("Ints") {
		REQUIRE (definitely_disjunct<Ints<0, 3>, Ints<4, 6>>);
		REQUIRE_FALSE (definitely_disjunct<Ints<0, 3>, Ints<3, 6>>);
		REQUIRE_FALSE (definitely_disjunct<Ints<3, 6>, Ints<0, 3>>);
		REQUIRE_FALSE (definitely_disjunct<Ints<3, 6>, Int>);
		REQUIRE_FALSE (definitely_disjunct_t<Ints_t<3, 6>, Union_t<Int_t>>::eval());
		REQUIRE (definitely_disjunct_t<Ints_t<3, 6>, Union_t<Nil_t>>::eval());
		REQUIRE_FALSE (definitely_disjunct_t<Byte_t, Int32_t>::eval());
	} SECTION ("Domains") {
		REQUIRE (definitely_disjunct<domain<some<Basic::String>>, Int>);
	}
}

TEST_CASE ("restrict") {
	REQUIRE_NOTHROW (restrict<int, Byte> {0});
	REQUIRE_THROWS (restrict<int, Byte> {-1});
	REQUIRE_NOTHROW (restrict<any, Byte> {0});
	REQUIRE_THROWS (restrict<any, Byte> {-1});
}

TEST_CASE ("unify_types") {
	REQUIRE (std::is_same_v<unify_types<int, int>, int>);
	REQUIRE (std::is_same_v<unify_types<int, double>, var>);
	REQUIRE (std::is_same_v<unify_types<some<Basic::String>, some<Boxed<int>>>, some<Interface::Value>>);
	REQUIRE (std::is_same_v<unify_types<unique<Basic::String>, shared<Basic::String>>, some<Basic::String>>);
	REQUIRE (std::is_same_v<unify_types<maybe<Basic::String>, shared<Basic::String>>, maybe<Basic::String>>);
}

TEST_CASE ("any") {
	SECTION ("Constructor") {
		REQUIRE (any{} == nullptr);
		REQUIRE (any {nullptr} == nullptr);
		REQUIRE (any {true} == true);
		REQUIRE (any {0} == 0);
		REQUIRE (any {1.0} == 1.0);
		REQUIRE (any {'a'} == 'a');
		REQUIRE (any {"String"} == "String");
		REQUIRE (any {any {"String"}} == "String");
		REQUIRE (any {static_cast<const any&> (any {"String"})} == "String");
		REQUIRE (any {var {"String"}} == "String");
		REQUIRE (any {static_cast<const var&> (var {"String"})} == "String");
		REQUIRE (any {some<> {"String"}} == "String");
		REQUIRE (any {static_cast<const some<>&> (some<> {"String"})} == "String");
	}

	SECTION ("Reassignment") {
		any x = nullptr;
		REQUIRE (x == nullptr);
		x = true;
		REQUIRE (x == true);
		x = 0;
		REQUIRE (x == 0);
		x = 1.0;
		REQUIRE (x == 1.0);
		x = 'a';
		REQUIRE (x == 'a');
		x = "String";
		REQUIRE (x == "String");
		x = std::move (x);
		REQUIRE (x == "String");
		x = x;
		REQUIRE (x == "String");
		x = some<> {"String2"};
		REQUIRE (x == "String2");
	}
}

TEST_CASE ("var") {
	SECTION ("Constructor") {
		REQUIRE (var {nullptr} == nullptr);
		REQUIRE (var {true} == true);
		REQUIRE (var {0} == 0);
		REQUIRE (var {1.0} == 1.0);
		REQUIRE (var {'a'} == 'a');
		REQUIRE (var {"String"} == "String");
		REQUIRE (var {var {"String"}} == "String");
		REQUIRE (var {static_cast<const var&> (var {"String"})} == "String");
		REQUIRE (var {any {"String"}} == "String");
	} SECTION ("Reassignment") {
		var x = nullptr;
		REQUIRE (x == nullptr);
		x = true;
		REQUIRE (x == true);
		x = 0;
		REQUIRE (x == 0);
		x = 1.0;
		REQUIRE (x == 1.0);
		x = 'a';
		REQUIRE (x == 'a');
		x = "String";
		REQUIRE (x == "String");
		x = std::move (x);
		REQUIRE (x == "String");
		x = x;
		REQUIRE (x == "String");
		x = any {"String"};
		REQUIRE (x == "String");
	}
}

TEST_CASE ("some/maybe") {
	SECTION ("Constructor") {
		REQUIRE (some<> {true} == true);
		REQUIRE (some<> {1} == 1);
		REQUIRE (some<> {1.0} == 1.0);
		REQUIRE (some<> {'a'} == 'a');
		REQUIRE (some<> {"String"} == "String");
		REQUIRE (some<> {any {"String"}} == "String");
		REQUIRE (some<> {static_cast<const any&>(any {"Heap Allocated String"})} == "Heap Allocated String");
	}

	SECTION ("Maybe") {
		REQUIRE (maybe<> {nullptr} == nullptr);
		REQUIRE (maybe<> {any {nullptr}} == nullptr);
		REQUIRE (maybe<> {1} == 1);
	}

	SECTION ("Reassignment") {
		maybe<> x = nullptr;
		REQUIRE (x == nullptr);
		x = true;
		REQUIRE (x == true);
		x = 0;
		REQUIRE (x == 0);
		x = 1.0;
		REQUIRE (x == 1.0);
		x = 'a';
		REQUIRE (x == 'a');
		x = "String";
		REQUIRE (x == "String");
		x = std::move (x);
		REQUIRE (x == "String");
		x = x;
		REQUIRE (x == "String");
		x = any {"any"};
		REQUIRE (x == "any");
		x = var {"var"};
		REQUIRE (x == "var");
	}

	SECTION ("Typed") {
		REQUIRE (some<Basic::String> {"String"} == "String");
		REQUIRE (some<Basic::String> {some<> {"String"}} == "String");
		REQUIRE (some<Basic::String> {any {"String"}} == "String");
	}
}

TEST_CASE ("unique") {
	SECTION ("Constructor") {
		REQUIRE (unique<> {true} == true);
		REQUIRE (unique<> {1} == 1);
		REQUIRE (unique<> {1.0} == 1.0);
		REQUIRE (unique<> {'a'} == 'a');
		REQUIRE (unique<> {"String"} == "String");
		REQUIRE (unique<> {unique<> {"String"}} == "String");
		REQUIRE (
				unique<> {static_cast<const unique<>&>(unique<> {"Heap Allocated String"})} == "Heap Allocated String");
	}

	SECTION ("Reassignment") {
		unique<Interface::Value, maybe_nil> x = nullptr;
		REQUIRE (x == nullptr);
		x = true;
		REQUIRE (x == true);
		x = 0;
		REQUIRE (x == 0);
		x = 1.0;
		REQUIRE (x == 1.0);
		x = 'a';
		REQUIRE (x == 'a');
		x = "String";
		REQUIRE (x == "String");
		x = std::move (x);
		REQUIRE (x == "String");
		x = x;
		REQUIRE (x == "String");
		x = any {"any"};
		REQUIRE (x == "any");
		x = var {"var"};
		REQUIRE (x == "var");
		decltype (x) other {"Other"};
		x = other;
		REQUIRE (x == "Other");
		REQUIRE (other == "Other");
	}
}
TEST_CASE ("shared") {
	SECTION ("Constructor") {
		REQUIRE (shared<> {true} == true);
		REQUIRE (shared<> {1} == 1);
		REQUIRE (shared<> {1.0} == 1.0);
		REQUIRE (shared<> {'a'} == 'a');
		REQUIRE (shared<> {"String"} == "String");
		REQUIRE (shared<> {shared<> {"String"}} == "String");
		REQUIRE (
				shared<> {static_cast<const shared<>&>(shared<> {"Heap Allocated String"})} == "Heap Allocated String");
	}

	SECTION ("Reassignment") {
		shared<Interface::Value, maybe_nil> x = nullptr;
		REQUIRE (x == nullptr);
		x = true;
		REQUIRE (x == true);
		x = 0;
		REQUIRE (x == 0);
		x = 1.0;
		REQUIRE (x == 1.0);
		x = 'a';
		REQUIRE (x == 'a');
		x = "String";
		REQUIRE (x == "String");
		x = std::move (x);
		REQUIRE (x == "String");
		x = x;
		REQUIRE (x == "String");
		x = any {"any"};
		REQUIRE (x == "any");
		x = var {"var"};
		REQUIRE (x == "var");
		decltype (x) other {"Other"};
		x = other;
		REQUIRE (x == "Other");
		REQUIRE (other == "Other");
	}
}

TEST_CASE ("immediate") {
	REQUIRE (immediate<Boxed<int>> {1} == 1);
	REQUIRE (immediate<Basic::String, 16> {"String"} == "String");
	REQUIRE_THROWS (immediate<Basic::String, 0> {"String"});
}

TEST_CASE ("Persistent::Vector") {
	SECTION ("Construction") {
		std::vector<char32_t> y {'H', 'e', 'l', 'l', 'o'};
		unique<Persistent::Vector<var>> x = y;

		REQUIRE (x == y);
		REQUIRE (VEC (1, 2, 3) == VEC (1, 2, 3));
		auto a = VEC (1, 2, 3, 4, 5, 6, 7);
		REQUIRE (a == a);
	} SECTION ("append") {
		unique<Persistent::Vector<int>> x = VEC ();
		x = append (x, 1);
		REQUIRE (first (x) == 1);
	}
}

TEST_CASE ("Primitive Sets") {
	SECTION ("Nil") {
		REQUIRE (Nil (nullptr));
		REQUIRE (Nil (maybe<> {nullptr}));
		REQUIRE_FALSE (Nil (false));
	} SECTION ("Bool") {
		REQUIRE (Bool (true));
		REQUIRE (Bool (any {true}));
		REQUIRE (any {Bool} (true));
		REQUIRE_FALSE (Bool (nullptr));
		REQUIRE (True (true));
		REQUIRE_FALSE (True (false));
	} SECTION ("Int") {
		REQUIRE (Int (0));
		REQUIRE_FALSE (Int (0.0));
		REQUIRE (Byte (255));
		REQUIRE_FALSE (Byte (256));
	} SECTION ("Double") {
		REQUIRE (Double (0.0));
		REQUIRE (Double (0.0f));
		REQUIRE_FALSE (Double (0));
	} SECTION ("Character") {
		REQUIRE (Character ('a'));
		REQUIRE (Character (U'a'));
		REQUIRE_FALSE (Character (24));
	} SECTION ("String") {
		REQUIRE (String ("Hello"));
		REQUIRE (String (any {"Hello"}));
		REQUIRE (String (string {"Hello"}));
	}
}

TEST_CASE ("STR") {
	REQUIRE (STR ("a") == "a");
	REQUIRE (any {STR ("a")} == "a");
	REQUIRE (unique<> {STR ("a")} == "a");
}

TEST_CASE ("to_string") {
	REQUIRE (to_string (42) == "42");
	REQUIRE (to_string (1.1) == "1.1");
	REQUIRE (to_string ("Hello") == "Hello");
	REQUIRE (to_string (STR ("x")) == "x");
	REQUIRE (to_string ('x') == "x");
	REQUIRE (to_string (VEC ('x', "x", STR ("x"))) == "['x', \"x\", \"x\"]");
	REQUIRE (to_string (tuple<int, int> (1, 2)) == "[1, 2]");
}

TEST_CASE ("Object Casting") {
	struct A {};
	struct B {};

	REQUIRE_NOTHROW (obj_cast<const A&> (any {A {}}));
	REQUIRE_THROWS (obj_cast<const A&> (any {B {}}));

	shared<> x (A {});
	REQUIRE_NOTHROW (obj_cast<A&> (x));
	weak<> x_weak = x;
	REQUIRE_NOTHROW (obj_cast<const A&> (x_weak));
	REQUIRE_THROWS (obj_cast<A&> (x_weak));
	shared<> x_copy = x;
	REQUIRE_THROWS (obj_cast<A&> (x));
	REQUIRE_NOTHROW (obj_cast<const A&> (x));
}

TEST_CASE ("utf8") {
	REQUIRE (utf8::read_char (u8"\U000000D8") == U'\U000000D8');
}

TEST_CASE ("concat") {
	REQUIRE (concat ("Hello ", "World") == "Hello World");
	REQUIRE (concat (VEC (1), VEC (2)) == VEC (1, 2));
}

TEST_CASE ("Interface::Exception") {
	struct Test_Exception : public Interface::Exception {
		const char* what () const noexcept override { return "Test Error"; }
	};

	try {
		throw Test_Exception ();
	}
	catch (Interface::Exception& e) {
		REQUIRE (e.error_message () == "Test Error");
	}
}

TEST_CASE ("Type Classes") {
	struct _ {
		static int test_function (int) { return 0; }
	};

	auto test_function = _::test_function;

	REQUIRE (std::is_same_v<type_class<int>, Type_Class::Int>);
	REQUIRE (std::is_same_v<type_class<std::tuple<int, float>>, Type_Class::Tuple<int, float>>);
	REQUIRE (std::is_same_v<type_class<decltype (stdout)>, Type_Class::File_Stream>);
	REQUIRE (std::is_same_v<type_class<decltype ("Hello World")>, Type_Class::CString>);
	REQUIRE (
			std::is_same_v<type_class<std::vector<int>>, Type_Class::Iterable<Type_Class::Iterable_Capabilities<true, true, true, true>>>);
	REQUIRE (
			std::is_same_v<type_class<std::unordered_map<int, int>>, Type_Class::Iterable<Type_Class::Iterable_Capabilities<true, false, false, false>>>);
	REQUIRE (std::is_same_v<type_class<decltype (test_function)>, Type_Class::Function<int, int>>);
}

TEST_CASE ("map filter reduce") {
	REQUIRE (map ([] (int x) { return x * 2; }, VEC (1, 2, 3)) == VEC (2, 4, 6));

	REQUIRE (filter (Int, VEC (1, 'a', 2, "Hello", 3)) == VEC (1, 2, 3));

	REQUIRE (reduce ([] (auto&& v, auto&& e) { return append (FORWARD (v), FORWARD (e)); }, VEC (), VEC (1, 2, 3)) ==
			 VEC (1, 2, 3));
}

TEST_CASE ("generic_enumerator") {
	using namespace pure;
	std::vector<int> v {1, 2, 3, 4};
	auto e1 = iterator_range<decltype (v.begin ())> {v};
	REQUIRE (e1.has_size ());
	REQUIRE (e1.size () == 4);
	generic_enumerator e {iterator_range<decltype (v.begin ())> {v.begin (), v.end ()}};
	auto e2 (e);

	REQUIRE (e2.has_size ());
	REQUIRE (e2.size () == 4);


	{
		std::vector<int> test {1, 2, 3, 4};
		std::vector<std::vector<int>> v {};
		v.emplace_back (std::move (test));

		auto e = move_iterator_range<decltype (v.begin ())> {v.begin (), v.end ()};

		auto v2 = e.move ();

		REQUIRE (v[0].size () == 0);
	}
}

TEST_CASE ("Basic::String") {
	using namespace pure;
	unique<Basic::String> x = "Hello World";
	auto y = make_unique<Basic::String> (with_capacity, 120, "Hi");
	REQUIRE (y->capacity () == 120);
}

TEST_CASE ("boxed") {
	using namespace pure;
	auto x = make_unique<Boxed<int64_t>> (42);

	REQUIRE (x->get_int64 () == 42);
}
using namespace pure;

TEST_CASE ("From Var") {
	bool b = any {true};
	REQUIRE (b);
	int i = any {-2};
	REQUIRE (i == -2);
	double d = any {1.0};
	REQUIRE (d == 1.0);
	char32_t c = any {'a'};
	REQUIRE (c == 'a');

	any s_ {"Hello"};
	const char* s = s_;
	REQUIRE (std::strcmp (s, "Hello") == 0);
}

TEST_CASE ("compare") {
	REQUIRE (0 == any {0});
	REQUIRE (any (1) == any {true});

	REQUIRE (equal ("Hello", any {"Hello"}));
}

TEST_CASE ("Arithmetic") {
	REQUIRE (any (3) + any (8) == 11);
}

TEST_CASE ("Functional") {
	static_assert (Trait_Functional<char>::implemented == false);
	REQUIRE (set ("Hello", 1, U'\U000000D8') == u8"H\U000000D8llo");
	static_assert (Trait_Functional<any>::implemented);
	REQUIRE (arity (any ("Hello")) == 1);
	REQUIRE (set (any ("Hello"), 1, U'\U000000D8') == u8"H\U000000D8llo");
	REQUIRE (set (any (""), 0, 'a') == "a");

	REQUIRE (apply (u8"Hell\U000000D8", 4) == U'\U000000D8');

	REQUIRE (apply (some<Basic::String> ("Hello World"), 4) == 'o');
	REQUIRE (apply (some<> ("Hello World"), 4) == 'o');

	unique<> str = "Hello";
	REQUIRE (set (str, 0, 'h') == "hello");
}

TEST_CASE ("Enumerable") {
	REQUIRE (Enumerable ("Hello"));
	REQUIRE (!Empty ("Hello"));
	REQUIRE (Empty (""));
	REQUIRE (nth ("Hello", 4) == 'o');
	REQUIRE (append ("Hell", 'o') == "Hello");

	REQUIRE (Enumerable (any ("Hello")));
	REQUIRE (!Empty (any ("Hello")));
	REQUIRE (Empty (any ("")));
	REQUIRE (nth (any ("Hello"), 4) == 'o');
	REQUIRE (append (any ("Hell"), 'o') == "Hello");

	REQUIRE (Enumerable (unique<> ("Hello")));
	REQUIRE (!Empty (unique<> ("Hello")));
	REQUIRE (Empty (unique<> ("")));
	REQUIRE (nth (unique<> ("Hello"), 4) == 'o');
	REQUIRE (append (unique<> ("Hell"), 'o') == "Hello");
}

TEST_CASE ("fopen") {
	IO::print_to (IO::fopen ("test.txt", "w"), "Hello World");
}

TEST_CASE ("Error") {
	REQUIRE (to_string (operation_not_supported ()));
	REQUIRE (Error (operation_not_supported ()));
	REQUIRE (Error (any (operation_not_supported ())));
}

TEST_CASE ("Function") {
	any f = [] (int x) { return 2 * x; };

	any g = [] (const auto& x) { return 2 * x; };

	REQUIRE (apply (f, 2) == 4);
}

TEST_CASE ("Tuple") {
	REQUIRE (equal (tuple<> (), tuple<> ()));
	REQUIRE (equal (tuple<int> (1), tuple<int> (1)));
	REQUIRE (!equal (tuple<int> (1), tuple<int> (2)));
	REQUIRE (!equal (tuple<int> (1), tuple<> ()));
	REQUIRE (!equal (tuple<> (), ""));

	REQUIRE (pure::apply (tuple<int, double> (1, 2.0), 0) == 1);
	REQUIRE (pure::apply (tuple<int, double> (1, 2.0), 1) == 2.0);

	REQUIRE (count (tuple<int> (1)) == 1);
	REQUIRE (nth (tuple<int> (1), 0) == 1);

	generic_enumerator e = enumerate (tuple<int, double> (1, 2.0));
	REQUIRE (e.read () == 1);
	e.next ();
	REQUIRE (e.read () == 2.0);

	any x = tuple<> ();
	REQUIRE (Empty (x));

	REQUIRE (nth (any (tuple<int> (1)), 0) == 1);
}

TEST_CASE ("Iterable") {
	auto a = std::vector<int> {1, 2, 3, 4};

	REQUIRE (Enumerable (a));

	REQUIRE (equal (a, a));
	REQUIRE (equal (a, std::tuple<int, int, int, int> (1, 2, 3, 4)));
	REQUIRE (to_string (a) == "[1, 2, 3, 4]");

	REQUIRE (to_string (any (a)) == "[1, 2, 3, 4]");
}

TEST_CASE ("Record") {
	auto x = make_record (STR ("x"), 1, STR ("y"), 2);
	REQUIRE (x.nth_id (1) == STR ("y"));

	REQUIRE (first (x) == std::make_tuple (STR ("x"), 1));
	REQUIRE (x (STR ("y")) == 2);
	#if !defined (PURE_COMPILER_MSVC)
	REQUIRE (pure::apply (x, STR ("y")) == 2);
	// For some reason this triggers an internal error in MSVC
	#endif
	REQUIRE (equal (x, x));

	auto y = make_record (STR ("y"), 2, STR ("x"), 1);
	REQUIRE (x == y);
	REQUIRE (to_string (x) == "{\"x\" : 1, \"y\" : 2}");
	any x_ = x;
	REQUIRE (x_ == x);
}

TEST_CASE ("Constructors") {
	auto a = MAP (0, 1, 2, 3);
	auto b = MAP (STR ("x"), 0, STR("y"), 1);

	REQUIRE (append (VEC (0), 1) == VEC (0, 1));
	REQUIRE (set (a, 0, 4) (0) == 4);
}

TEST_CASE ("Readme") {
	#if !defined (PURE_COMPILER_MSVC)
	SECTION ("Sets") {
		REQUIRE (Nil (nullptr));
		REQUIRE (Nil (0) == false);

		REQUIRE (Bool (true));
		REQUIRE (Int (123));

		REQUIRE (Ints <0, 10> (7));

		REQUIRE (Double (123.0));
		REQUIRE (Character (U'ß'));

		REQUIRE (String ("Hello World"));
		REQUIRE (String (STR ("ns::name")));

		REQUIRE (Vector<Ints <1, 3>> (VEC (1, 2, 3)));
		REQUIRE (Vector<Any> (VEC (1, "2", '3')));

		REQUIRE (Tuple<Int, String, Character> (VEC (1, "2", '3')));

		REQUIRE (Function<String, Int> (MAP ("a", 1, "b", 2, "c", 3)));
		REQUIRE (Record<STR ("name"), String, STR ("age"), Int> (MAP (STR ("name"), "Albert",
															 STR ("age"), 99)));

		REQUIRE (Set<Any> (Set<Any>));

		// Every boolean function is a set
		REQUIRE (Set<Any> ([] (int x) -> bool {return x % 2 == 0;}));

		REQUIRE (Error (operation_not_supported()));
		REQUIRE (Object (stdout));

		REQUIRE (Any (nullptr));
		REQUIRE (None (nullptr) == false);
	}
	#endif

	SECTION ("Domains") {
		REQUIRE (domain<unique<Basic::String>> ("Hello World"));
		REQUIRE_THROWS (restrict <any, Ints<0, 10>> {42});

	}

	SECTION ("Basic Operations") {

		REQUIRE (equal (true, 1.0) == false);

		REQUIRE (VEC (1, 2, 3) < VEC (1, 2, 4));
		REQUIRE (equivalent_compare (VEC (1, 2, 3), VEC (1, 2, 4)) == -1);

		bool result = VEC (1, 2, 3) != "1, 2, 3";
		REQUIRE (result);
	}

	SECTION ("Functional") {
		// apply applies some arguments to a functional value
		#if ! defined (PURE_COMPILER_MSVC)
		// MSVC really doesn't like apply, causes internal errors
		REQUIRE (apply (Int, 1) == true);
		REQUIRE (apply (compare, 3, 5) == -1);
		REQUIRE (apply (MAP ("a", 1, "b", 2), "b") == 2);
		REQUIRE (pure::apply (VEC (1, 2, 3), 1) == 2);
		REQUIRE (apply ("Hello", 2) == 'l');
		#endif

		// Functions, sets (and some vectors) also forward the call operator to
		// apply
		REQUIRE (Int (1) == true);
		REQUIRE (compare (3, 5) == -1);

		REQUIRE (MAP ("a", 1, "b", 2) ("b") == 2);

		// set (f, key, value) returns a new value in which key is mapped to value
		REQUIRE (set (MAP (), "a", 1) == MAP ("a", 1));
		REQUIRE (set (VEC (nullptr, 2, 3), 0, 1) == VEC (1, 2, 3));
		REQUIRE (set ("Hello", 4, U'\U000000D8') == u8"Hell\U000000D8");
	}

	SECTION ("Enumerable") {
		REQUIRE (Enumerable ("Hello World"));
		REQUIRE (Enumerable (VEC (1, 2, 3)));
		REQUIRE (Enumerable (MAP ("a", 1, "b", 2)));
		REQUIRE (Enumerable ([] (const auto& x) {return x + 1;}) == false);
		REQUIRE (Enumerable (Any) == false);

		// Elements of enumerable values can be accessed through nth
		REQUIRE (nth (VEC (1, 2, 3), 0) == 1);
		REQUIRE (first (VEC (1, 2, 3)) == 1);
		REQUIRE (second (VEC (1, 2, 3)) == 2);

		// count returns the number of elements in an enumerable value
		REQUIRE (count (VEC (1, 2, 3)) == 3);
		REQUIRE (count (MAP ("a", 1, "b", 2)) == 2);
		REQUIRE (Empty (VEC()));

		// append (v, element) returns a vector with element added to v
		REQUIRE (append (VEC (1, 2), 3) == VEC (1, 2, 3));
		REQUIRE (append (VEC (1, 2, 3), "End") == VEC (1, 2, 3, "End"));

		// concat (v1, v2) returns a vector that is the concatenation of v1 and v2
		REQUIRE (concat (VEC (1, 2), VEC (3, 4)) == VEC (1, 2, 3, 4));

		// Functional style iteration
		REQUIRE (map ([] (auto&& x) {return x * 2;}, VEC (1, 2, 3)) == VEC (2, 4, 6));

		REQUIRE (filter (Int, VEC (1, 1.0, 2, "Hello World", 3)) == VEC (1, 2, 3));

		REQUIRE (reduce ([] (int result, int next) {return result + next;}, 0, VEC(1, 2, 3)) == 6);

		// There's also support for traditional style iteration through the
		// enumerator interface
		auto accumulate = [] (const auto& vec, auto result) {
			for (auto e = enumerate (vec); !e.empty(); e.next()) {
				result += e.read();
			}
			return result;
		};

		REQUIRE (accumulate (VEC (1, 2, 3), 0) == 6);
	}

	SECTION ("Formatting") {
		REQUIRE (to_string (VEC (1, 2, 3)) == "[1, 2, 3]");
		REQUIRE (to_string (MAP (STR("a"), 1, STR("b"), 2)) == "{\"a\" : 1, \"b\" : 2}");
	}

	SECTION ("Records") {
		auto point = MAP (STR ("x"), 1.0, STR ("y"), 2.0);

		REQUIRE (point ("x") == 1.0);
		REQUIRE (point (STR ("y")) == 2.0); // Compile-time lookup through type of STR ("y")

		// Sets can be used to check 'type'
		#if defined (PURE_COMPILER_MSVC)
		auto Point_2D = Record_t <STR_t ("x"), Double_t, STR_t ("y"), Double_t> {};
		#else
		auto Point_2D = Record <STR ("x"), Double, STR ("y"), Double>;
		#endif
		REQUIRE (Point_2D (point));
	}

}