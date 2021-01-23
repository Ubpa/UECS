#pragma once

#include <array>
#include <string_view>

namespace Ubpa::UECS {
    constexpr std::size_t hash_combine(std::size_t x, std::size_t y) noexcept;
    template<std::size_t N>
    constexpr std::size_t hash_combine(const std::array<std::size_t, N>& arr) noexcept;
    template<typename Container>
    std::size_t hash_combine(const Container& container) noexcept;
}

#include "Util.inl"
