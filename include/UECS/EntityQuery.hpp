#pragma once

#include "ArchetypeFilter.hpp"
#include "CmptLocator.hpp"
#include "details/Util.hpp"

namespace Ubpa::UECS {
	// ArchetypeFilter + CmptLocator
	struct EntityQuery {
		ArchetypeFilter filter;
		CmptLocator locator;

		std::size_t GetValue() const noexcept;

		bool IsMatch(const small_flat_set<TypeID>& types) const noexcept;
	};
	
	bool operator==(const EntityQuery& lhs, const EntityQuery& rhs) noexcept;
}

template<>
struct std::hash<Ubpa::UECS::EntityQuery> {
	std::size_t operator()(const Ubpa::UECS::EntityQuery& query) const noexcept {
		return query.GetValue();
	}
};
