#pragma once

#include <UTemplate/TypeID.h>

namespace Ubpa {
    constexpr size_t hash_combine(size_t x, size_t y) noexcept {
        return x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
    }
}
