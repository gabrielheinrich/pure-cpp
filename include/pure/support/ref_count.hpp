#pragma once

#include <atomic>
#include <cstdint>

namespace pure::detail {
	struct ref_counted {
		std::atomic<intptr_t> ref_count;

		constexpr ref_counted () : ref_count {intptr_t (1)} {}
		constexpr ref_counted (const ref_counted&) : ref_count {intptr_t (1)} {}
	};

	static inline void inc_ref_count (ref_counted& self) {
		std::atomic_fetch_add_explicit (&self.ref_count, intptr_t (1), std::memory_order_relaxed);
	}

	template<typename T>
	T* inc_ref_count_and_get (T& other) {
		inc_ref_count (other);
		return &other;
	}

	static inline bool dec_ref_count (ref_counted& self) {
		if (std::atomic_fetch_sub_explicit (&self.ref_count, intptr_t (1), std::memory_order_release) > 1)
			return false;
		std::atomic_thread_fence (std::memory_order_acquire);
		return true;
	}

	template<typename T>
	void dec_ref_count_and_delete (T& other) {
		if (dec_ref_count (other)) delete (&other);
	}

	static inline bool ref_count_is_unique (const ref_counted& self) {
		if (std::atomic_load_explicit (&self.ref_count, std::memory_order_relaxed) == 1)
			return std::atomic_load_explicit (&self.ref_count, std::memory_order_acquire) == 1;
		else
			return false;
	}
}