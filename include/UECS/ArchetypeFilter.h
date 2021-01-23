#pragma once

#include "AccessTypeID.h"

#include <set>

namespace Ubpa::UECS {
	// filter Archetype with all, any and none
	struct ArchetypeFilter {
		AccessTypeIDSet all;
		AccessTypeIDSet any;
		std::set<TypeID> none;

		std::size_t GetValue() const noexcept;

		bool HaveWriteTypeID() const noexcept;

		bool operator==(const ArchetypeFilter& rhs) const noexcept;
	};
}
