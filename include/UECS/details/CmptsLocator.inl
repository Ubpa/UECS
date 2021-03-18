#pragma once

#include <UTemplate/Func.hpp>

namespace Ubpa::UECS::details {
	template<typename... Cmpts>
	CmptLocator GenerateCmptLocator(TypeList<Cmpts...>) {
		if constexpr (sizeof...(Cmpts) > 0) {
			constexpr std::array types{ AccessTypeID_of<Cmpts>... };
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
		return details::GenerateCmptLocator(CmptList{});
	}

	template<typename Func>
	CmptLocator& CmptLocator::Combine() {
		CmptLocator funcLocator = Generate<Func>();
		for (const auto& type : funcLocator.cmptTypes)
			cmptTypes.insert(type);
		UpdateGetValue();
		return *this;
	}
}
