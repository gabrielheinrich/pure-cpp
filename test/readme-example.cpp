#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-comparison"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#if defined (_MSC_VER)
#pragma warning (disable : 4553 4834 4521)
#endif

#include <pure/core.hpp>

// Not strictly necessary but highly recommended
using namespace pure;

#ifdef PURE_COMPILER_GCC
int main() {

	// ******************************************************
	// # Builtin Categories
	// ******************************************************

	// Nil
	auto n = nullptr;
	// Bool
	auto b = true;
	// Int
	auto i = 0;
	// Double
	auto d = 0.0;
	// Character
	auto c = 'a';

	// String
	auto s = "String";
	auto s_id = STR ("String"); // Compile-time constexpr

	// Vector
	auto v = VEC (1, true, "Hello World");

	// Function
	auto f = [] (auto&& name) { return concat ("Hello ", FORWARD(name)); };
	auto m = MAP (STR ("a"), 1, STR ("b"), 2, STR ("c"), 3);

	// Set
	auto S = [] (const auto& x) -> bool { return x < 10 && (x % 2 == 0); };

	// Error
	auto e = operation_not_supported(); // Error

	// Object
	auto o = IO::fopen ("test.txt", "w");

	// ******************************************************
	// # Sets
	// ******************************************************

	// Sets are named in uppercase by convention
	// They are called like functions
	Nil (nullptr);
	Nil (0) == false;

	Bool (true);
	Int (123);

	// Range of Integers 0, 1, .. 10
	Ints <0, 10> (7);

	Double (123.0);
	Character (U'ß');

	String ("Hello World");
	String (STR ("ns::name"));

	Vector<Ints <1, 3>> (VEC (1, 2, 3));
	Vector<Any> (VEC (1, "2", '3'));

	Tuple<Int, String, Character> (VEC (1, "2", '3'));

	Function<String, Int> (MAP ("a", 1, "b", 2, "c", 3));
	Record<STR ("name"), String, STR ("age"), Int> (MAP (STR ("name"), "Albert",
														 STR ("age"), 99));

	Set<Any> (Set<Any>);

	// Every boolean function is a set
	Set<Any> ([] (int x) -> bool {return x % 2 == 0;});

	Error (operation_not_supported());
	Object (stdout);

	Any (nullptr);
	None (nullptr) == false;

	// ******************************************************
	// # Gradual typesystem
	// ******************************************************

	// any is the most basic type to hold arbitrary values
	any x = "Hello World";
	any x_1 = 42;
	any x_2 = VEC ("Hello", "World");
	any x_3 = stdout;

	// There are multiple other types, which all inherit from any, but add
	// further restrictions

	// uniquely owned heap allocated Object of unknown type
	unique<> x_unique = "Hello World";

	// ref counted heap allocated Object of unknown type
	shared<> x_shared = "Hello World";

	// Object of type Basic::String with dynamically handled ownership
	some<Basic::String> x_string = "Hello World";

	// Some Basic::String or nullptr
	maybe<Basic::String> x_maybe_string = nullptr;

	// Allocated on the stack
	immediate<Boxed<const char*>> x_cstring = "Hello World";

	// 'fully' typed string
	unique<Basic::String> x_unique_string = "Hello World";

	// var is similar to any but is 128bit in size and can therefore allocate
	// 64bit integers and double values inplace
	var x_var = 2.0;

	// ******************************************************
	// # Domains
	// ******************************************************

	// Every type has a domain, which is a constexpr set
	domain<unique<Basic::String>> ("Hello World"); // true

	// A type's domain can be restricted
	restrict <any, String> x_restricted = "Hello World"; // Ok

	// Domains can be used for static and dynamic checking

	// This wouldn't compile because domain<int> and String don't overlap
	// restrict <any, String> x_restricted = 42; Static Error

	// This will compile, but throw an exception at runtime
	try {
		restrict <any, Ints<0, 10>> x_dyn_restricted = 42; // Dynamic Error
	}
	catch (...) { }

	// ******************************************************
	// # Basic Operations
	// ******************************************************

	// operators == != < > <= >= + - * / % work as known from C++
	true == 1.0; // true

	// equal checks strict value equality
	equal (true, 1.0) == false;

	// Vectors and strings are also comparable
	VEC (1, 2, 3) < VEC (1, 2, 4);
	compare (VEC (1, 2, 3), VEC (1, 2, 4)) == -1;

	// Comparison operators can be used accross categories
	VEC (1, 2, 3) != "1, 2, 3";

	// ******************************************************
	// # Functional
	// ******************************************************

	// All strings, vectors, functions and sets are functional values

	// apply applies some arguments to a functional value
	apply (Int, 1) == true;
	apply (compare, 3, 5) == -1;
	apply (MAP ("a", 1, "b", 2), "b") == 2;
	pure::apply (VEC (1, 2, 3), 1) == 2;
	apply ("Hello", 2) == 'l';

	// Functions, sets (and some vectors) also forward the call operator to
	// apply
	Int (1) == true;
	compare (3, 5) == -1;

	MAP ("a", 1, "b", 2) ("b") == 2;

	// set (f, key, value) returns a new value in which key is mapped to value
	set (MAP (), "a", 1) == MAP ("a", 1);
	set (VEC (nullptr, 2, 3), 0, 1) == VEC (1, 2, 3);
	set ("Hello", 4, U'ø') == "Hellø";

	// ******************************************************
	// # Enumerable
	// ******************************************************

	// All strings and vectors and some functions and sets are enumerable
	Enumerable ("Hello World");
	Enumerable (VEC (1, 2, 3));
	Enumerable (MAP ("a", 1, "b", 2));
	Enumerable ([] (const auto& x) {return x + 1;}) == false;
	Enumerable (Any) == false;

	// Elements of enumerable values can be accessed through nth
	nth (VEC (1, 2, 3), 0) == 1;
	first (VEC (1, 2, 3)) == 1;
	second (VEC (1, 2, 3)) == 2;

	// count returns the number of elements in an enumerable value
	count (VEC (1, 2, 3)) == 3;
	count (MAP ("a", 1, "b", 2)) == 2;
	Empty (VEC());

	// append (v, element) returns a vector with element added to v
	append (VEC (1, 2), 3) == VEC (1, 2, 3);
	append (VEC (1, 2, 3), "End") == VEC (1, 2, 3, "End");

	// concat (v1, v2) returns a vector that is the concatenation of v1 and v2
	concat (VEC (1, 2), VEC (3, 4)) == VEC (1, 2, 3, 4);

	// Functional style iteration
	map ([] (auto&& x) {return x * 2;}, VEC (1, 2, 3)) == VEC (2, 4, 6);

	filter (Int, VEC (1, 1.0, 2, "Hello World", 3)) == VEC (1, 2, 3);

	reduce ([] (int result, int next) {return result + next;}, 0, VEC(1, 2, 3)) == 6;

	// There's also support for traditional style iteration through the
	// enumerator interface
	auto accumulate = [] (const auto& vec, auto result) {
		for (auto e = enumerate (vec); !e.empty(); e.next()) {
			result += e.read();
		}
		return result;
	};

	accumulate (VEC (1, 2, 3), 0) == 6;

	// ******************************************************
	// # Formatting
	// ******************************************************

	// Values are printed in a JSON style
	to_string (VEC (1, 2, 3)) == "[1, 2, 3]";
	to_string (MAP (STR ("a"), 1, STR("b"), 2)) == "{\"a\" : 1, \"b\" : 2}";

	// ******************************************************
	// # Records
	// ******************************************************

	// Enumerable functions, which only have strings as keys are called
	// records.

	// If MAP is called with compile-time strings as keys, it returns an
	// efficient tuple-like struct.

	// Here the memory layout of the return value is equivalent to
	// struct {double x; double y;}
	auto point = MAP (STR ("x"), 1.0, STR ("y"), 2.0);

	point ("x") == 1.0;
	point (STR ("y")) == 2.0; // Compile-time lookup through type of STR ("y")

	// Sets can be used to check 'type'
	auto Point_2D = Record <STR ("x"), Double, STR ("y"), Double>;
	Point_2D (point);

	// ******************************************************
	// # IO
	// ******************************************************

	// By convention all functions, which 'do' something, i.e. have
	// side-effects, should be in the namespace IO

	IO::print ("Hello World!");
	IO::print_to (IO::fopen("/tmp/test_file", "w"), "Hello temporary file!");

	return 0;
}

#else
int main() {
		// Nil
	auto n = nullptr;
	// Bool
	auto b = true;
	// Int
	auto i = 0;
	// Double
	auto d = 0.0;
	// Character
	auto c = 'a';

	// String
	auto s = "String";
	auto s_id = STR ("String"); // Compile-time constexpr

	// Vector
	auto v = VEC (1, true, "Hello World");

	// Function
	auto f = [] (auto&& name) { return concat ("Hello ", FORWARD(name)); };
	auto m = MAP (STR ("a"), 1, STR ("b"), 2, STR ("c"), 3);

	// Set
	auto S = [] (const auto& x) -> bool { return x < 10 && (x % 2 == 0); };

	// Error
	auto e = operation_not_supported(); // Error

	// Object
	auto o = IO::fopen ("test.txt", "w");

	// ******************************************************
	// # Sets
	// ******************************************************

	// Sets are named in uppercase by convention
	// They are called like functions
	Nil (nullptr);
	Nil (0) == false;

	Bool (true);
	Int (123);

	// Range of Integers 0, 1, .. 10
	Ints <0, 10> (7);

	Double (123.0);
	Character (U'\U000000df');

	String ("Hello World");
	String (STR ("ns::name"));

	Vector_t<Ints_t <1, 3>>{} (VEC (1, 2, 3));
	Vector_t<Any_t>{} (VEC (1, "2", '3'));

	Tuple_t<Int_t, String_t, Character_t>{} (VEC (1, "2", '3'));

	Function_t<String_t, Int_t>{} (MAP ("a", 1, "b", 2, "c", 3));
	Record_t<STR_t ("name"), String_t, STR_t ("age"), Int_t>{} (MAP (STR ("name"), "Albert",
														 STR ("age"), 99));

	Set_t<Any_t>{} (Set_t<Any_t>{});

	// Every boolean function is a set
	Set_t<Any_t>{} ([] (int x) -> bool {return x % 2 == 0;});

	Error (operation_not_supported());
	Object (stdout);

	Any (nullptr);
	None (nullptr) == false;

	// ******************************************************
	// # Gradual typesystem
	// ******************************************************

	// any is the most basic type to hold arbitrary values
	any x = "Hello World";
	any x_1 = 42;
	any x_2 = VEC ("Hello", "World");
	any x_3 = stdout;

	// There are multiple other types, which all inherit from any, but add
	// further restrictions

	// uniquely owned heap allocated Object of unknown type
	unique<> x_unique = "Hello World";

	// ref counted heap allocated Object of unknown type
	shared<> x_shared = "Hello World";

	// Object of type Basic::String with dynamically handled ownership
	some<Basic::String> x_string = "Hello World";

	// Some Basic::String or nullptr
	maybe<Basic::String> x_maybe_string = nullptr;

	// Allocated on the stack
	immediate<Boxed<const char*>> x_cstring = "Hello World";

	// 'fully' typed string
	unique<Basic::String> x_unique_string = "Hello World";

	// var is similar to any but is 128bit in size and can therefore allocate
	// 64bit integers and double values inplace
	var x_var = 2.0;

	// ******************************************************
	// # Domains
	// ******************************************************

	// Every type has a domain, which is a constexpr set
	domain<unique<Basic::String>> ("Hello World"); // true

	// A type's domain can be restricted
	restrict <any, String> x_restricted = "Hello World"; // Ok

	// Domains can be used for static and dynamic checking

	// This wouldn't compile because domain<int> and String don't overlap
	// restrict <any, String> x_restricted = 42; Static Error

	// This will compile, but throw an exception at runtime
	try {
		restrict <any, Ints<0, 10>> x_dyn_restricted = 42; // Dynamic Error
	}
	catch (...) { }

	// ******************************************************
	// # Basic Operations
	// ******************************************************

	// operators == != < > <= >= + - * / % work as known from C++
	true == 1.0; // true

	// equal checks strict value equality
	equal (true, 1.0) == false;

	// Vectors and strings are also comparable
	VEC (1, 2, 3) < VEC (1, 2, 4);
	compare (VEC (1, 2, 3), VEC (1, 2, 4)) == -1;

	// Comparison operators can be used accross categories
	VEC (1, 2, 3) != "1, 2, 3";

	// ******************************************************
	// # Functional
	// ******************************************************

	// All strings, vectors, functions and sets are functional values

	// apply applies some arguments to a functional value
	apply (Int, 1) == true;
	apply (compare, 3, 5) == -1;
	apply (MAP ("a", 1, "b", 2), "b") == 2;
	pure::apply (VEC (1, 2, 3), 1) == 2;
	apply ("Hello", 2) == 'l';

	// Functions, sets (and some vectors) also forward the call operator to
	// apply
	Int (1) == true;
	compare (3, 5) == -1;

	MAP ("a", 1, "b", 2) ("b") == 2;

	// set (f, key, value) returns a new value in which key is mapped to value
	set (MAP (), "a", 1) == MAP ("a", 1);
	set (VEC (nullptr, 2, 3), 0, 1) == VEC (1, 2, 3);
	set ("Hello", 4, U'\U000000D8') == "Hellø";

	// ******************************************************
	// # Enumerable
	// ******************************************************

	// All strings and vectors and some functions and sets are enumerable
	Enumerable ("Hello World");
	Enumerable (VEC (1, 2, 3));
	Enumerable (MAP ("a", 1, "b", 2));
	Enumerable ([] (const auto& x) {return x + 1;}) == false;
	Enumerable (Any) == false;

	// Elements of enumerable values can be accessed through nth
	nth (VEC (1, 2, 3), 0) == 1;
	first (VEC (1, 2, 3)) == 1;
	second (VEC (1, 2, 3)) == 2;

	// count returns the number of elements in an enumerable value
	count (VEC (1, 2, 3)) == 3;
	count (MAP ("a", 1, "b", 2)) == 2;
	Empty (VEC());

	// append (v, element) returns a vector with element added to v
	append (VEC (1, 2), 3) == VEC (1, 2, 3);
	append (VEC (1, 2, 3), "End") == VEC (1, 2, 3, "End");

	// concat (v1, v2) returns a vector that is the concatenation of v1 and v2
	concat (VEC (1, 2), VEC (3, 4)) == VEC (1, 2, 3, 4);

	// Functional style iteration
	map ([] (auto&& x) {return x * 2;}, VEC (1, 2, 3)) == VEC (2, 4, 6);

	filter (Int, VEC (1, 1.0, 2, "Hello World", 3)) == VEC (1, 2, 3);

	reduce ([] (int result, int next) {return result + next;}, 0, VEC(1, 2, 3)) == 6;

	// There's also support for traditional style iteration through the
	// enumerator interface
	auto accumulate = [] (const auto& vec, auto result) {
		for (auto e = enumerate (vec); !e.empty(); e.next()) {
			result += e.read();
		}
		return result;
	};

	accumulate (VEC (1, 2, 3), 0) == 6;

	// ******************************************************
	// # Formatting
	// ******************************************************

	// Values are printed in a JSON style
	to_string (VEC (1, 2, 3)) == "[1, 2, 3]";
	to_string (MAP (STR ("a"), 1, STR("b"), 2)) == "{\"a\" : 1, \"b\" : 2}";

	// ******************************************************
	// # Records
	// ******************************************************

	// Enumerable functions, which only have strings as keys are called
	// records.

	// If MAP is called with compile-time strings as keys, it returns an
	// efficient tuple-like struct.

	// Here the memory layout of the return value is equivalent to
	// struct {double x; double y;}
	auto point = MAP (STR ("x"), 1.0, STR ("y"), 2.0);

	point ("x") == 1.0;
	point (STR ("y")) == 2.0; // Compile-time lookup through type of STR ("y")

	// Sets can be used to check 'type'
	auto Point_2D = Record_t <STR_t ("x"), Double_t, STR_t ("y"), Double_t> {};
	Point_2D (point);

	// ******************************************************
	// # IO
	// ******************************************************

	// By convention all functions, which 'do' something, i.e. have
	// side-effects, should be in the namespace IO

	IO::print ("Hello World!");
	IO::print_to (IO::fopen("test_file", "w"), "Hello temporary file!");

	return 0;
}
#endif