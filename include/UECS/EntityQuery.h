#pragma once

#include "EntityFilter.h"
#include "EntityLocator.h"

namespace Ubpa {
	class EntityQuery {
	public:
		template<typename... AllCmpts, typename... AnyCmpts, typename... NoneCmpts, typename... Cmpts>
		EntityQuery(TypeList<AllCmpts...>, TypeList<AnyCmpts...>, TypeList<NoneCmpts...>, TypeList<Cmpts...>);

		EntityQuery(EntityFilter filter, EntityLocator locator);

		size_t HashCode() const noexcept { return hashCode; }

		const EntityFilter& Filter() const noexcept { return filter; }

		const EntityLocator& Locator() const noexcept { return locator; }
	private:
		EntityFilter filter;
		EntityLocator locator;
		size_t hashCode;
	};
}

#include "detail/EntityQuery.inl"
