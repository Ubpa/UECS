#pragma once

#include "Util.h"

namespace Ubpa {
	template<typename... AllCmpts, typename... AnyCmpts, typename... NoneCmpts>
	EntityFilter::EntityFilter(TypeList<AllCmpts...>, TypeList<AnyCmpts...>, TypeList<NoneCmpts...>)
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
	void EntityFilter::InsertAll(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			allCmptTypes.insert(type);
		allHashCode = GenAllHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void EntityFilter::InsertAny(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			anyCmptTypes.insert(type);
		anyHashCode = GenAnyHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void EntityFilter::InsertNone(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			noneCmptTypes.insert(type);
		noneHashCode = GenNoneHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void EntityFilter::EraseAll(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			allCmptTypes.erase(type);
		allHashCode = GenAllHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void EntityFilter::EraseAny(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			anyCmptTypes.erase(type);
		anyHashCode = GenAnyHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename CmptTypeContainer>
	void EntityFilter::EraseNone(const CmptTypeContainer& c) {
		if (c.empty())
			return;
		for (const auto& type : c)
			noneCmptTypes.erase(type);
		noneHashCode = GenNoneHashCode();
		combinedHashCode = GenCombinedHashCode();
	}

	template<typename... CmptTypes, typename>
	void EntityFilter::InsertAll(CmptTypes... types) {
		static_assert(sizeof...(CmptTypes) > 0);
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		InsertAll(typeArr.data(), typeArr.size());
	}

	template<typename... CmptTypes, typename>
	void EntityFilter::InsertAny(CmptTypes... types) {
		static_assert(sizeof...(CmptTypes) > 0);
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		InsertAny(typeArr.data(), typeArr.size());
	}

	template<typename... CmptTypes, typename>
	void EntityFilter::InsertNone(CmptTypes... types) {
		static_assert(sizeof...(CmptTypes) > 0);
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		InsertNone(typeArr.data(), typeArr.size());
	}

	template<typename... CmptTypes, typename>
	void EntityFilter::EraseAll(CmptTypes... types) {
		static_assert(sizeof...(CmptTypes) > 0);
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		EraseAll(typeArr.data(), typeArr.size());
	}

	template<typename... CmptTypes, typename>
	void EntityFilter::EraseAny(CmptTypes... types) {
		static_assert(sizeof...(CmptTypes) > 0);
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		EraseAny(typeArr.data(), typeArr.size());
	}

	template<typename... CmptTypes, typename>
	void EntityFilter::EraseNone(CmptTypes... types) {
		static_assert(sizeof...(CmptTypes) > 0);
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		EraseNone(typeArr.data(), typeArr.size());
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
