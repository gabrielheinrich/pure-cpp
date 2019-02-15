#pragma once

#include <cstdint>

namespace pure::detail::murmur3 {
	constexpr uint32_t c1 = 0xcc9e2d51;
	constexpr uint32_t c2 = 0x1b873593;

	inline uint32_t rotl (uint32_t x, int8_t r) {
		return (x << r) | (x >> (32 - r));
	}

	inline uint32_t mix_k1 (uint32_t k1) {
		k1 *= c1;
		k1 = rotl (k1, 15);
		return k1 * c2;
	}

	inline uint32_t mix_h1 (uint32_t h1, uint32_t k1) {
		h1 ^= k1;
		h1 = rotl (h1, 13);
		return h1 * 5 + 0xE6546B64;
	}

	inline uint32_t fmix (uint32_t h) {
		h ^= h >> 16;
		h *= 0x85ebca6b;
		h ^= h >> 13;
		h *= 0xc2b2ae35;
		h ^= h >> 16;

		return h;
	}

	inline int32_t hash_raw_bytes (const void* key, int len, uint32_t seed) {
		auto data = reinterpret_cast<const uint8_t*>(key);
		const int num_blocks = len / 4;

		uint32_t h1 = seed;

		auto blocks = reinterpret_cast<const uint32_t*>(data + num_blocks * 4);

		for (int i = -num_blocks; i; ++i) {
			uint32_t k1 = mix_k1 (blocks[i]);
			h1 = mix_h1 (h1, k1);
		}

		auto tail = reinterpret_cast<const uint8_t*>(data + num_blocks * 4);

		uint32_t k1 = 0;

		switch (len & 3) {
			case 3: k1 ^= tail[2] << 16;
			case 2: k1 ^= tail[1] << 8;
			case 1: k1 ^= tail[0];
				k1 *= c1;
				k1 = rotl (k1, 15);
				k1 *= c2;
				h1 ^= k1;
		};

		h1 ^= len;
		return fmix (h1);
	}

	inline int32_t hash_cstring (const char* str, uint32_t seed) {
		return hash_raw_bytes (str, std::strlen (str), seed);
	}

	inline int32_t hash_int32 (int32_t value, uint32_t seed) {
		auto h1 = mix_h1 (seed, mix_k1 (value));
		return fmix (mix_h1 (h1, 0));
	}

	inline int32_t hash_int64 (int64_t value, uint32_t seed) {
		uint32_t low = static_cast<uint32_t> (value);
		uint32_t high = static_cast<uint32_t> (value >> 32);

		auto h1 = mix_h1 (seed, mix_k1 (low));
		return fmix (mix_h1 (h1, high));
	}

	inline int32_t hash_combine_ordered (uint32_t a, uint32_t b) {
		return a ^ (b + 0x9E3779B9 + (a << 6) + (a >> 2));
	}

	inline int32_t hash_combine_unordered (uint32_t a, uint32_t b) {
		return (a ^ b) + 0x9E3779B9;
	}

	inline int32_t hash_mix (uint32_t a, uint32_t b) {
		auto k1 = mix_k1 (a);
		return fmix (mix_h1 (b, k1));
	}
}