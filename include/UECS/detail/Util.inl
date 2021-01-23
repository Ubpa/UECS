#pragma once

#include <UTemplate/TypeID.h>

namespace Ubpa::UECS {
    constexpr std::size_t hash_combine(std::size_t x, std::size_t y) noexcept {
        return x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
    }

    template<std::size_t N>
    constexpr std::size_t hash_combine(const std::array<std::size_t, N>& arr) noexcept {
        static_assert(N >= 2);
        std::size_t rst = arr[0];
        for (std::size_t i = 1; i < N; i++)
            rst = hash_combine(rst, arr[i]);
        return rst;
    }

    template<typename Container>
    std::size_t hash_combine(const Container& container) noexcept {
        static_assert(std::is_same_v<typename Container::value_type, std::size_t>);
        assert(!container.empty());
        auto iter = container.cbegin();
        std::size_t rst = *iter;
        ++iter;
        while (iter != container.cend()) {
            rst = hash_combine(rst, *iter);
            ++iter;
        }
        return rst;
    }
}
