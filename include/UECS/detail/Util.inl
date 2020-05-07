#pragma once

#include <UTemplate/TypeID.h>

namespace Ubpa {
    constexpr size_t hash_combine(size_t x, size_t y) noexcept {
        return x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
    }

    template<size_t N>
    inline constexpr size_t hash_combine(const std::array<size_t, N>& arr) noexcept {
        static_assert(N >= 2);
        size_t rst = arr[0];
        for (size_t i = 1; i < N; i++)
            rst = hash_combine(rst, arr[i]);
        return rst;
    }

    template<typename Container>
    size_t hash_combine(const Container& container) noexcept {
        static_assert(std::is_same_v<typename Container::value_type, size_t>);
        assert(!container.empty());
        auto iter = container.cbegin();
        size_t rst = *iter;
        ++iter;
        while (iter != container.cend()) {
            rst = hash_combine(rst, *iter);
            ++iter;
        }
        return rst;
    }
}
