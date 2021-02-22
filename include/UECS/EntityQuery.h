#pragma once

#include "ArchetypeFilter.h"
#include "CmptLocator.h"
#include "details/Util.h"

namespace Ubpa::UECS {
	// ArchetypeFilter + CmptLocator
	struct EntityQuery {
		ArchetypeFilter filter;
		CmptLocator locator;

		std::size_t GetValue() const noexcept { return hash_combine(filter.GetValue(), locator.GetValue()); }

		bool operator==(const EntityQuery& query) const noexcept {
			return filter == query.filter && locator == query.locator;
		}
	};
}

template<>
struct std::hash<Ubpa::UECS::EntityQuery> {
	std::size_t operator()(const Ubpa::UECS::EntityQuery& query) const noexcept {
		return query.GetValue();
	}
};
