#pragma once

namespace Ubpa {
	template<typename... AllCmpts, typename... AnyCmpts, typename... NoneCmpts, typename... Cmpts>
	EntityQuery::EntityQuery(TypeList<AllCmpts...>, TypeList<AnyCmpts...>, TypeList<NoneCmpts...>, TypeList<Cmpts...>)
		: filter{ TypeList<AllCmpts...>{}, TypeList<AnyCmpts...>{}, TypeList<NoneCmpts...>{} },
		locator{ TypeList<Cmpts...>{} }
	{
	}
}

namespace std {
	template<typename T>
	struct hash;
	template<>
	struct hash<Ubpa::EntityQuery> {
		size_t operator()(const Ubpa::EntityQuery& query) const noexcept {
			return query.HashCode();
		}
	};
}
