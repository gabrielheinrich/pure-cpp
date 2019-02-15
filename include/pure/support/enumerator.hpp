#pragma once

#include <new>

namespace pure {
	namespace Interface {
		/**
		 * Interface::Enumerator
		 */
		template<typename T>
		struct Enumerator {
			using value_type = T;

			virtual ~Enumerator () {};

			virtual void copy_construct (void* memory, intptr_t size) const = 0;
			virtual void move_construct (void* memory, intptr_t size)&& { return copy_construct (memory, size); }

			/**
			 * next
			 */
			virtual void next () = 0;

			/**
			 * empty
			 */
			virtual bool empty () const noexcept = 0;

			/**
			 * read
			 */
			virtual const value_type& read () const = 0;

			/**
			 * move
			 */
			virtual value_type move () { return read (); }

			/**
			 * has_size
			 */
			virtual bool has_size () const { return false; }

			/**
			 * size
			 */
			virtual intptr_t size () const { throw operation_not_supported (); }
		};
	}

	template<typename T, typename Value_Type = typename T::value_type>
	struct Boxed_Enumerator : Interface::Enumerator<Value_Type> {
		using value_type = Value_Type;
		T self;

		template<typename... Args>
		Boxed_Enumerator (Args&& ... args) : self {std::forward<Args> (args)...} {}
		Boxed_Enumerator (const T& other) : self {other} {}
		Boxed_Enumerator (T&& other) : self {std::move (other)} {}

		void copy_construct (void* memory, intptr_t size) const override {
			assert (size >= sizeof (Boxed_Enumerator));
			new (memory) Boxed_Enumerator {self};
		}
		void move_construct (void* memory, intptr_t size)&& override {
			assert (size >= sizeof (Boxed_Enumerator));
			new (memory) Boxed_Enumerator {std::move (self)};
		}

		void next () override { self.next (); }
		bool empty () const noexcept override { return self.empty (); }

		const value_type& read () const override { return self.read (); }
		value_type move () override { return self.move (); }

		bool has_size () const override { return self.has_size (); }
		intptr_t size () const override { return self.size (); }
	};

	template<typename T>
	struct enumerator_base {
		using value_type = T;

		constexpr bool has_size () const { return false; }
		intptr_t size () const { throw operation_not_supported (); }
	};

	template<typename T>
	struct generic_enumerator_t;

	struct alignas (16) generic_enumerator : enumerator_base<any> {
		using value_type = any;
		char memory[12 * sizeof (intptr_t)];

		template<typename T, typename = std::enable_if_t<!std::is_same_v<generic_enumerator, std::decay_t<T>>>>
		generic_enumerator (T&& other) {
			using enumerator_type = typename generic_enumerator_t<std::decay_t<T>>::value;
			static_assert (std::is_base_of_v<Interface::Enumerator<any>, enumerator_type>);
			static_assert (sizeof (enumerator_type) <= sizeof (*this));
			new (this) enumerator_type {std::forward<T> (other)};
		}

		generic_enumerator (const generic_enumerator& other) {
			other.get ().copy_construct (this, sizeof (*this));
		}

		generic_enumerator (generic_enumerator&& other) {
			std::move (other.get ()).move_construct (this, sizeof (*this));
		}

		generic_enumerator& operator= (generic_enumerator& other) = delete;

		const Interface::Enumerator<any>&
		get () const { return *reinterpret_cast<const Interface::Enumerator<any>*> (this); }
		Interface::Enumerator<any>& get () { return *reinterpret_cast<Interface::Enumerator<any>*> (this); }

		bool empty () const noexcept { return get ().empty (); }
		void next () { return get ().next (); }
		const any& read () const { return get ().read (); }
		any move () { return get ().move (); }

		bool has_size () const { return get ().has_size (); }
		intptr_t size () const { return get ().size (); }
	};

	namespace detail {
		template<typename Iterator, typename Sentinel, typename = void>
		struct iterator_has_size : std::false_type {};

		template<typename Iterator, typename Sentinel>
		struct iterator_has_size<Iterator, Sentinel, decltype (std::declval<Sentinel> () -
															   std::declval<Iterator> (), void ())> : std::true_type {
		};
	}

	template<typename Iterator, typename Sentinel = Iterator>
	struct iterator_range : enumerator_base<typename Iterator::value_type> {
		using value_type = typename Iterator::value_type;

		Iterator begin;
		Sentinel end;

		iterator_range (Iterator begin, Sentinel end) : begin {std::move (begin)}, end {std::move (end)} {}

		template<typename Iterable>
		iterator_range (Iterable&& iterable) : begin {iterable.begin ()}, end {iterable.end ()} {}

		iterator_range (iterator_range& other) : iterator_range {static_cast<const iterator_range&>(other)} {}
		iterator_range (const iterator_range& other) : begin {other.begin}, end {other.end} {}
		iterator_range (iterator_range&& other) : begin {std::move (other.begin)}, end {std::move (other.end)} {}

		bool empty () const noexcept { return begin == end; }
		void next () { ++begin; }
		const value_type& read () const { return *begin; }
		value_type move () { return *begin; }

		static constexpr bool has_size () { return detail::iterator_has_size<Iterator, Sentinel>::value; }
		intptr_t size () const {
			if constexpr (has_size ())
				return end - begin;
			else
				throw operation_not_supported ();
		}
	};

	template<typename Iterator, typename Sentinel = Iterator>
	struct move_iterator_range : iterator_range<Iterator, Sentinel> {
		using value_type = typename iterator_range<Iterator, Sentinel>::value_type;
		using iterator_range<Iterator, Sentinel>::iterator_range;

		value_type move () { return std::move (*(this->begin)); }
	};
}

// inline implementations
#include <pure/object/interface.hpp>
#include <pure/types/immediate.hpp>
#include <pure/object/boxed.hpp>

namespace pure {
	template<typename T>
	struct any_ref_enumerator : T {
		using value_type = any;

		mutable immediate<Boxed<typename T::value_type*>> current;

		template<typename... Args>
		any_ref_enumerator (Args&& ... args) : T {std::forward<Args> (args)...}, current {} {}

		any_ref_enumerator (any_ref_enumerator& other)
				: T {static_cast<const T&>(other)}, current {} {}

		any_ref_enumerator (const any_ref_enumerator& other)
				: T {static_cast<const T&>(other)}, current {} {}

		any_ref_enumerator (any_ref_enumerator&& other)
				: T {static_cast<T&&>(other)}, current {} {}

		void next () {
			static_cast<T&>(*this).next ();
		}

		const any& read () const {
			current->self = const_cast<typename T::value_type*>(&(static_cast<const T&>(*this).read ()));
			return current;
		}

		any move () { return static_cast<T&>(*this).move (); }
	};

	template<typename T>
	struct any_box_enumerator : T {
		using value_type = any;
		mutable var current;

		template<typename... Args>
		any_box_enumerator (Args&& ... args) : T {std::forward<Args> (args)...}, current {nullptr} {}

		any_box_enumerator (const any_box_enumerator& other)
				: T {static_cast<const T&>(other)}, current {other.current} {}

		any_box_enumerator (any_box_enumerator&& other)
				: T {static_cast<T&&>(other)}, current {std::move (other.current)} {}

		void next () {
			current = nullptr;
			static_cast<T&>(*this).next ();
		}

		const any& read () const {
			if (current.tag () == Var::Tag::Nil) {
				current = static_cast<const T&> (*this).read ();
			}
			return current;
		}

		var move () {
			if (current.tag () != Var::Tag::Nil) {
				return std::move (current);
			}
			return static_cast<T&> (*this).move ();
		}
	};

	template<typename T>
	using any_enumerator =
	std::conditional_t<std::is_reference_v<decltype (std::declval<T> ().read ())>,
			std::conditional_t<std::is_base_of_v<any, typename T::value_type>, T, any_ref_enumerator<T>>,
			any_box_enumerator<T>>;

	template<typename T>
	struct unique_enumerator : enumerator_base<any> {
		using value_type = any;

		T* self;

		template<typename... Args>
		unique_enumerator (Args&& ... args) : self {new T {std::forward<Args> (args)...}} {}
		unique_enumerator (const unique_enumerator& other) : self {new T {*(other.self)}} {}
		unique_enumerator (unique_enumerator&& other) : self {other.self} { other.self = nullptr; }

		~unique_enumerator () { delete self; }

		bool empty () const noexcept { return self->empty (); }
		void next () { self->next (); }
		const value_type& read () const { return self->read (); }
		value_type move () { return self->move (); }

		bool has_size () const { return self->has_size (); }
		intptr_t size () const { return self->size (); }
	};

	template<typename T>
	struct generic_enumerator_t {
		using value = std::conditional_t<
				sizeof (Boxed_Enumerator<any_enumerator<T>, any>) <= sizeof (generic_enumerator),
				Boxed_Enumerator<any_enumerator<T>, any>, Boxed_Enumerator<unique_enumerator<any_enumerator<T>>, any>>;

	};
}
