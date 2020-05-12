#pragma once

#include "Util.h"

namespace Ubpa {
	template<typename... AllCmpts, typename... AnyCmpts, typename... NoneCmpts>
	EntityFilter::EntityFilter(TypeList<AllCmpts...>, TypeList<AnyCmpts...>, TypeList<NoneCmpts...>)
		: allCmptTypes{ CmptType::Of<AllCmpts>()... },
		anyCmptTypes{ CmptType::Of<AnyCmpts>()... },
		noneCmptTypes{ CmptType::Of<NoneCmpts>()... },
		allHashCode{ GenAllHashCode() },
		anyHashCode{ GenAnyHashCode() },
		noneHashCode{ GenNoneHashCode() },
		combinedHashCode{ GenCombinedHashCode() }
	{
		static_assert(sizeof...(AnyCmpts) != 1);
	}

	template<typename Container>
	void EntityFilter::InsertAll(const Container& c) {
		if (c.empty())
			return;
		for(const auto& type : c)
			allCmptTypes.insert(type);
		allHashCode = GenAllHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename Container>
	void EntityFilter::InsertAny(const Container& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			anyCmptTypes.insert(type);
		anyHashCode = GenAnyHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename Container>
	void EntityFilter::InsertNone(const Container& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			noneCmptTypes.insert(type);
		noneHashCode = GenNoneHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename Container>
	void EntityFilter::EraseAll(const Container& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			allCmptTypes.erase(type);
		allHashCode = GenAllHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename Container>
	void EntityFilter::EraseAny(const Container& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			anyCmptTypes.erase(type);
		anyHashCode = GenAnyHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename Container>
	void EntityFilter::EraseNone(const Container& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			noneCmptTypes.erase(type);
		noneHashCode = GenNoneHashCode();
		combinedHashCode = GenCombinedHashCode();
	}
}

namespace std {
	template<typename T>
	struct hash;
	template<>
	struct hash<Ubpa::EntityFilter> {
		size_t operator()(const Ubpa::EntityFilter& filter) const noexcept {
			return filter.HashCode();
		}
	};
}
