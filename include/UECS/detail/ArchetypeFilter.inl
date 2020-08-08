#pragma once

#include "Util.h"

namespace Ubpa::UECS {
	template<typename... AllCmpts, typename... AnyCmpts, typename... NoneCmpts>
	ArchetypeFilter::ArchetypeFilter(TypeList<AllCmpts...>, TypeList<AnyCmpts...>, TypeList<NoneCmpts...>)
		: allCmptTypes{ CmptType::Of<AllCmpts>... },
		anyCmptTypes{ CmptType::Of<AnyCmpts>... },
		noneCmptTypes{ CmptType::Of<NoneCmpts>... },
		allHashCode{ GenAllHashCode() },
		anyHashCode{ GenAnyHashCode() },
		noneHashCode{ GenNoneHashCode() },
		combinedHashCode{ GenCombinedHashCode() }
	{
		static_assert(sizeof...(AnyCmpts) != 1);
		static_assert(IsSet_v<TypeList<AllCmpts..., AnyCmpts..., NoneCmpts...>>);
	}

	template<typename CmptTypeContainer>
	void ArchetypeFilter::InsertAll(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			allCmptTypes.insert(type);
		allHashCode = GenAllHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void ArchetypeFilter::InsertAny(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			anyCmptTypes.insert(type);
		anyHashCode = GenAnyHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void ArchetypeFilter::InsertNone(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			noneCmptTypes.insert(type);
		noneHashCode = GenNoneHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void ArchetypeFilter::EraseAll(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			allCmptTypes.erase(type);
		allHashCode = GenAllHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void ArchetypeFilter::EraseAny(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			anyCmptTypes.erase(type);
		anyHashCode = GenAnyHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void ArchetypeFilter::EraseNone(const CmptTypeContainer& c) {
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
	struct hash<Ubpa::UECS::ArchetypeFilter> {
		size_t operator()(const Ubpa::UECS::ArchetypeFilter& filter) const noexcept {
			return filter.HashCode();
		}
	};
}
