# Pure C++
[![Build Status](https://travis-ci.org/gabrielheinrich/pure-cpp.svg?branch=master)](https://travis-ci.org/gabrielheinrich/pure-cpp)

#### 'Within C++, there is a much smaller and cleaner language struggling to get out' - Bjarne Stroustrup


Pure C++ is a library for gradually-typed and data-oriented functional
programming in C++.  

In contrast to traditional typed functional programming, it proposes sets
instead of types as the fundamental ontological concept. Pure C++ is highly
influenced by the Clojure programming language.

## Disclaimer
Pure C++ is still in an experimental proof of concept stage. The library requires
the C++ 17 standard and is tested with GCC 8.1 Clang 9.0 and MSVC 15 2017.

## Features

#### Value semantics
C++ has a strong notion of objects and types. Pure C++ adds a strong notion of
values and sets.  The value universe is the union of disjunct base sets, called
categories. Every value is a member of exactly one category. A C++ object
state can model a value from that value universe by implementing a value's
logical properties. With that relation established, sets instead of types can
be used as the primary language to describe program behaviour.
    
#### Gradual typesystem
All types with overlapping domains can be implicitly converted into each
other, while value identity is preserved. The distinction between static and
dynamic typing is replaced by a gradual degree of specificity in restricting
the domain of an object.

#### Typeclass and Trait system
Meta-typesystem in which every type has a type class. All fundamental
operations are openly defined through traits, which can be specialized for
individual types or whole type classes. The system can be freely extended with
new implementations for user types.

#### Immutability and persistent data structures
Efficient implementation of immutability through move semantics and integration
of persistent data structures from the immer library.

#### C++ Integration
Integration of all iterable types as well as `std::tuple`, `std::pair`, 
`std::exception` ...

## Rationale

### Value Semantics
* Value semantics with a canonical value universe transcends differences
  between programming languages and more generally between programming and
  mathematics. It's a concise language to define what a program should do,
  without fixing how the computation should be performed.
* Types are freed of their semantic meaning. Code can be understood without
  knowledge of the types. Types can be used as a language to select
  implementations without interfering with the logical behaviour of the
  program.

### Gradual Typesystem
* All source code becomes syntactically equivalent to dynamically typed code,
  while the machine code stays specifically typed.
* There are use cases for general dynamic types in most large programs. Many
  large scale C++ projects already contain more or less well integrated
  implementations of dynamic types.
* A gradual typesystem is the logical conclusion of both static and dynamic
  typing. Every typesystem is inherently static. In a dynamic typesystem
  there's just only one static type. If a static typesystem contains a type,
  that any other type can be implicitly converted to and from, it can combine
  the virtues of both static and dynamic typing.

### Set-based domain specification through types
* The main purpose of types should be to specify details of the implementation
  of objects.
* Additionally every type also always defines a domain: a set of values that
  objects of that type can represent, which is restricted naturally by the
  capabilities of the implementation (memory layout, methods, invariants etc...)
* If the relation between a type and the underlying set, that is its domain is
  made explicit the gap is closed between traditional typed functional
  programming a la Haskell and mathematics, where sets are commonly used
  instead of types. In the mathematical interpretation of the program every
  type just has to be conceptually replaced with its domain.
* Many benefits arise :
    * clearer conceptual model
    * notion of logically equivalent types, namely types with equal domain
    * set algebra as a language is much smaller and more canonical than the
      type language.
    * foreign types can be quickly understood in terms of their domain
    * static type checking and dynamic assertions are unified under one
      concept, namely set membership.

### C++
* Huge library and tooling infrastructure, huge community.
* First class access to C APIs and ability to expose C APIs.
* High performance and zero-cost abstractions.
* Possibility to reach 'under the hood' for performance critical parts of a
  program.

## Usage

Pure C++ is a header only C++ 17 library. You should use cmake to install the
library on your machine
```
> git clone https://github.com/gabrielheinrich/pure-cpp
> cd pure-cpp
> mkdir build && cd build
> cmake ..
> make install
```
Afterwards you can use `find_package (PureCpp)` in your CMakeLists.txt files
```
find_package (PureCpp)
target_link_libraries (my_executable pure-cpp)
```

The main header, which will give you access to the full library is called
`<pure/core.hpp>`.  
It's highly recommended to add `using namespace pure;` to
your source files to use a concise syntax.

## Example

```cpp
#include <pure/core.hpp>

// Not strictly necessary but highly recommended
using namespace pure;

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

	// var is the most basic type to hold arbitrary values
	var x = "Hello World";
	var x_1 = 42;
	var x_2 = VEC ("Hello", "World");
	var x_3 = stdout;

	// There are multiple other types, which all inherit from var, but add
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

	// ******************************************************
	// # Domains
	// ******************************************************

	// Every type has a domain, which is a constexpr set
	domain<unique<Basic::String>> ("Hello World"); // true

	// A type's domain can be restricted
	restrict <var, String> x_restricted = "Hello World"; // Ok

	// Domains can be used for static and dynamic checking

	// This wouldn't compile because domain<int> and String don't overlap
	// restrict <var, String> x_restricted = 42; Static Error

	// This will compile, but throw an exception at runtime
	try {
		restrict <var, Ints<0, 10>> x_dyn_restricted = 42; // Dynamic Error
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
```

## Basic Concepts

### Objects and Values

* Objects are an area of memory within the execution environment. They have a
  type and a lifetime.
* Values are mathematical entities. They have neither a type nor a lifetime or
  an address and are therefore also not mutable. Even the commonly used term
  _immutable value_ is a misconception, as it gives the impression that there
  could in fact be mutable values, which is not the case.
* The logical properties of a value are defined in terms of fundamental
  operations, that can be applied to the value.
* Identity is the most fundamental property of a value. A value is defined by
  being different from all other values.
* C++ objects can model values. An object models a value, if calling
  fundamental operations on the object always returns results, that are
  coherent with the logical properties of the modelled value.
* For every type there's a set of all values that objects of that type can
  model. This set is called the domain of the type.

### Variables and Bindings
* The word _variable_ is misused in programming. In mathematics variables are not
  called variables because they represent something mutable, but because they
  are parameters to a function or logical statement. They are called variable,
  because a parametized mathematical term can be instantiated with many
  'variable' bindings of its paramters.
* To not intermix this notion of variability with that of mutability of objects
  any identifier that doesn't refer to an argument to a function, should not be
  called a variable, but a binding.
* In pure functional programming a binding is just a shorthand name given to a
  more complex expression. This is also true for globals.

### Value Holder system

* Pure C++ introduces a common base class for heap allocated objects called
  `Inferface::Value`
* `Interface::Value` contains a (maybe unused) reference count and a v-table
  to dispatch fundamental operations dynamically.
* For every type `T` there's a type `Boxed<T>`, which inherits from
  `Interface::Value`. It has a member object of type `T` and forwards all
  operations dynamically or statically to that object.
* Types that inherit from `Interface::Value` are not handled directly, but
  through smart pointers, also reffered to as value holders.
* All the builtin value holders inherit from a common base class called `var`.
  All smart pointers can thus bind to `const var&` and `var&&`
* Most smart pointers are pointer sized. They use the low zero bits of the
  pointer to distinguish different kinds of ownership (unique, shared, weak,
  moveable etc...). See : `some<T>, maybe<T>, unique<T>, shared<T>,
  interned<T>, weak<T>`
* There's support for inplace allocation of fundamental types like int,
  small strings etc.... See : `var`
* Objects don't have to be allocated separately on the heap, but can be
  allocated in conjunction with the smart pointer itself. See : `immediate<T>`

### Value universe & categories

* Categories are the base sets that constitute the value universe of Pure C++.
* Every value belongs to exactly one category.
* New categories can be defined by the user, while the canonical builtin
  categories are sufficient for most applications.

#### Atomic categories

##### Nil

The category Nil contains only one value, namely `nullptr`. It can be used as a
sentinel or to mark the abscence of a real value. The type `std::nullptr` has
the domain Nil.

##### Bool

The category Bool contains the values `true` and `false` and is used to specify
truth value. The C++ type `bool` has the domain Bool.

##### Int

Int is the set of all integers that fall in the range of a 2's complement 64bit
representation. All builtin signed and unsigned integer types are interpreted
as values of this category. To reduce complexity unsigned 64bit integers are
not supported at the moment.

##### Double

Double is the set of all 64bit floating point IEEE 754 numbers.  Objects of
type `float` and `double` model values of this category.

##### Character

Character is the set of all Unicode code points. The types `char`, `char16_t`
and `char32_t` represent values of this category.

#### Functional & enumerable categories

##### String

Strings are sequences of values from the category Character. They expose a
utf-8 encoded null-terminated byte string representation.  Objects of type
`const char*` are interpreted as Strings, which includes all C++ string
literals. Also values of type `std::string` belong to this category. There's a
macro `STR ("key")`, which returns a constexpr string of unique type, which can
be used as an efficient lookup key or other kind of marker.

##### Function

Functions represent mappings of values to other values. A function can be
applied to arguments to yield a new value.
Functions may or may not be enumerable. The enumeration of a function returns
tuples (i.e. vectors) which combine arguments and the value these arguments are
mapped to.
All C++ functions that don't have the return type `bool` belong to this
category, as well as hash maps and records. The abstract constructor for
enumerable functions has the form `MAP (key, value, ...)`.

##### Set

A Set is identifiecd by a function returning a boolean value, also called the
characteristic function of the set.  
Sets may or may not be enumerable. In contrast to regular functions their
enumeration just contains the argument tuples, for which the characteristic
function returns true. If the arity of the set is strictly one, the individual
values in the enumeration get unwrapped (instead of being represented as
vectors of size one).
All C++ functions with return type `bool` are interpreted as sets. Hash sets
and other data structures which offer a membership test method, belong to this
category. To easily distinguish sets from functions, names of sets are
capitalized in Pure C++, e.g. `Any`, `Int`, `Empty`, `Enumerable`,...  

##### Vector

Vectors are mappings from an integer range 0...n to some set. All vectors are
enumerable.  
All iterable types including `std::vector`, ranges (i.e. iterator pairs) as
well as `std::tuple`, `std::pair` and tuples in general belong to the category
Vector.  The abstract constructor for Vectors has the form `VEC (elements,...)`

#### Special categories

##### Error

Values of category Error represent some error condition and error message. All
types inheriting from `std::exception` belong to this category.
Values of category Error don't support equality comparison. They can be
returned from functions or thrown as exceptions.

##### Object

Any object, that doesn't fall into one of the other categories represents an
opaque value of category Object. This category is typically used to represent
handles to state carrying entities in the execution environment like streams,
global variables, databases, sockets, user interface windows etc...

## Contact

Pure C++ is being developed by Gabriel Heinrich. Contact : gabriel@pure-cpp.org
