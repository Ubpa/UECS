#pragma once

#include <UTemplate/Func.h>
#include <UContainer/Algorithm.h>

namespace Ubpa::UECS::detail {
	template<typename... Cmpts>
	CmptLocator GenerateCmptLocator(TypeList<Cmpts...>) {
		if constexpr (sizeof...(Cmpts) > 0) {
			constexpr std::array types{ CmptAccessType::Of<Cmpts>... };
			return CmptLocator{ types };
		}
		else
			return {};
	}
}

namespace Ubpa::UECS {
	template<typename Func>
	CmptLocator CmptLocator::Generate() {
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;
		using CmptList = Filter_t<ArgList, IsNonSingleton>;
		return detail::GenerateCmptLocator(CmptList{});
	}

	template<typename Func>
	CmptLocator& CmptLocator::Combine() {
		CmptLocator funcLocator = Generate<Func>();
		cmptTypes = SetUnion(cmptTypes, funcLocator.cmptTypes);
		UpdateHashCode();
		return *this;
	}
}
