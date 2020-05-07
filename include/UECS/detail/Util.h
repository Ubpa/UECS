#pragma once

#include <array>

namespace Ubpa {
    inline constexpr size_t hash_combine(size_t x, size_t y) noexcept;
    template<size_t N>
    inline constexpr size_t hash_combine(const std::array<size_t, N>& arr) noexcept;
    template<typename Container>
    inline size_t hash_combine(const Container& container) noexcept;
}

#include "Util.inl"
