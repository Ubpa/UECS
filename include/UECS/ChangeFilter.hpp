#pragma once

#include <USmallFlat/small_flat_set.hpp>

#include <UTemplate/Type.hpp>

namespace Ubpa::UECS {
    class ChangeFilter {
    public:
        small_flat_set<TypeID> types;
    };
}
