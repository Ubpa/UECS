#pragma once

#include "CmptType.h"

#include <UTemplate/TypeList.h>

#include <set>

namespace Ubpa::UECS {
	// filter Archetype with All, Any and None
	struct ArchetypeFilter {
		std::set<CmptType> all;
		std::set<CmptType> any;
		std::set<CmptType> none;

		size_t HashCode() const noexcept;

		bool operator==(const ArchetypeFilter& rhs) const;
	};
}
