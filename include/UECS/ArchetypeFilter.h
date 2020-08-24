#pragma once

#include "CmptType.h"

#include <UTemplate/TypeList.h>

#include <set>

namespace Ubpa::UECS {
	// filter Archetype with all, any and none
	struct ArchetypeFilter {
		std::set<CmptAccessType> all;
		std::set<CmptAccessType> any;
		std::set<CmptType> none;

		size_t HashCode() const noexcept;

		bool operator==(const ArchetypeFilter& rhs) const;
	};
}
