#pragma once

#include <exception>
#include <type_traits>
#include <pure/traits.hpp>
#include <pure/support/string_builder.hpp>
#include <pure/types/var.hpp>
#include <pure/support/file_stream.hpp>
#include <stdio.h>

namespace pure {
	template<typename T>
	void error (T&& e) {
		static_assert (std::is_base_of_v<std::exception, std::decay_t<T>>);
		throw std::forward<T> (e);
	}

	/**
	 Returns a readable string representation of self. Works on atomic and enumerable values.
	 */
	template<typename T>
	auto to_string (const T& self) {
		detail::string_builder result {127};
		IO::print_to (result, self);
		return result.finish ();
	}

	namespace IO {
		/**
		 Opens a file for reading or writing via std::fopen. The difference to std::fopen is that the returned FILE*
		 is wrapped in a type that automatically closes it once it goes out of scope.
		 */
		pure::file_stream fopen (const char* file_name, const char* mode) {
			#ifdef PURE_COMPILER_MSVC
				FILE* stream = nullptr;
				if (::fopen_s (&stream, file_name, mode)) throw operation_not_supported();
			#else
				FILE* stream = ::fopen (file_name, mode);
				if (stream == nullptr) throw operation_not_supported ();
			#endif
			return {stream};
		}
	}

	/**
	 Returns the concatenation of two values. Works on vectors and strings.
	 */
	template<typename A, typename B>
	auto concat (A&& lhs, B&& rhs) {
		static_assert (Trait_Enumerable<B>::implemented);
		var result = std::forward<A> (lhs);
		for (auto enumerator = enumerate (rhs); !enumerator.empty (); enumerator.next ()) {
			result = append (std::move (result), enumerator.read ());
		}
		return std::move (result);
	};

	template<typename Source, typename Fn>
	struct map_sequence : implements<Type_Class::Sequence> {

		using Source_Enumerator = decltype (pure::enumerate (std::declval<Source> ()));
		Source source;
		Fn map_fn;

		template<typename S, typename F>
		map_sequence (S&& source, F&& map_fn) : source (std::forward<S> (source)), map_fn (std::forward<F> (map_fn)) {}

		struct enumerator {
			const map_sequence& owner;
			Source_Enumerator source;

			using value_type = decltype (std::declval<Fn> () (std::declval<typename Source_Enumerator::value_type> ()));

			enumerator (const map_sequence& owner) :
					owner {owner},
					source {pure::enumerate (owner.source)} {}

			void next () { source.next (); }
			bool empty () const { return source.empty (); }
			value_type read () {
				return pure::apply (owner.map_fn, source.read ());
			}

			value_type move () {
				return pure::apply (owner.map_fn, source.move ());
			}

			bool has_size () const { return source.has_size (); }
			intptr_t size () const { return source.size (); }
		};

		enumerator enumerate () const { return {*this}; }
	};

	/**
	 Returns a (lazy) vector consisiting of the result of applying f to each of the items of vec
	 @param f A function of arity 1
	 @param vec A vector of items, to which f should be applied one by one.
	 */
	template<typename F, typename V>
	auto map (F&& f, V&& vec) {
		return map_sequence<V, F> {std::forward<V> (vec), std::forward<F> (f)};
	};

	template<typename Source, typename Fn>
	struct filter_sequence : implements<Type_Class::Sequence> {

		using Source_Enumerator = decltype (pure::enumerate (std::declval<Source> ()));
		Source source;
		Fn filter_fn;

		template<typename S, typename F>
		filter_sequence (S&& source, F&& filter_fn) : source (std::forward<S> (source)),
													  filter_fn (std::forward<F> (filter_fn)) {}

		struct enumerator {
			const filter_sequence& owner;
			Source_Enumerator source;

			using value_type = typename Source_Enumerator::value_type;

			enumerator (const filter_sequence& owner) :
					owner {owner},
					source {pure::enumerate (owner.source)} {}

			void next () { source.next (); }
			bool empty () const { return source.empty (); }
			value_type read () {
				while (!owner.filter_fn (source.read ()))
					source.next ();
				return source.read ();
			}

			value_type move () {
				auto e = source.move ();
				while (!owner.filter_fn (e)) {
					source.next ();
					e = source.move ();
				}
				return std::move (e);
			}

			bool has_size () const { return false; }
			intptr_t size () const { throw operation_not_supported (); }
		};

		enumerator enumerate () const { return {*this}; }
	};

	/**
	 Returns a (lazy) vector consisiting of all items in vec for which f returns true.
	 @param f A set, i.e. boolean function of arity 1, which specifies whether or not an item should be included in
	 the result.
	 @param vec A vector of items which should be filtered.
	 */
	template<typename F, typename V>
	auto filter (F&& f, V&& vec) {
		return filter_sequence<V, F> {std::forward<V> (vec), std::forward<F> (f)};
	};

	/**
	 Functional style looping. f has to be a function taking two arguments: An accumulated result and a next value
	 to apply to the result. reduce returns the result of applying f to all the values in vec, starting with an
	 initial value.
	 @param f The looping function (result, next) -> result
	 @param initial The initial value to feed to f.
	 @param vec The Vector to loop over.
	 */
	template<typename F, typename Initial, typename V>
	auto reduce (const F& f, Initial&& initial, V&& vec) {
		auto enumerator = enumerate (std::forward<V> (vec));
		using return_type = unify_types<Initial, decltype (f (initial, enumerator.read ()))>;

		if (enumerator.empty ()) return return_type {std::forward<Initial> (initial)};

		return_type result = f (std::forward<Initial> (initial), enumerator.read ());
		enumerator.next ();
		for (; enumerator.empty () == false; enumerator.next ()) {
			result = f (std::move (result), enumerator.read ());
		}

		return std::move (result);
	};
}
