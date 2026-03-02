#pragma once
#include <cstdint>

/**
 * @brief Utility struct providing hashing functions.
 *
 * This struct currently implements a constexpr version of the 64‑bit
 * FNV‑1a hash algorithm, suitable for hashing string literals or
 * compile‑time constant data.
 */
struct Hash
{
    /**
     * @brief Computes a 64‑bit FNV‑1a hash.
     *
     * This is a constexpr implementation of the FNV‑1a algorithm,
     * allowing the hash to be evaluated at compile time when the input
     * is known at compile time.
     *
     * @param data Pointer to the byte sequence to hash.
     * @param len  Number of bytes to process.
     *
     * @return std::uint64_t The resulting 64‑bit hash value.
     */
    static constexpr std::uint64_t FNV1aHash(const char* data, size_t len)
    {
        std::uint64_t hash = 14695981039346656037ull; // FNV offset basis
        for (size_t i = 0; i < len; ++i)
        {
            hash *= 1099511628211ull;                 // FNV prime
            hash xor_eq static_cast<std::uint8_t>(data[i]);
        }
        return hash;
    }
};