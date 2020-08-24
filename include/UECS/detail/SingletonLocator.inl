#pragma once

#include <UTemplate/Func.h>
#include <UContainer/Algorithm.h>

namespace Ubpa::UECS::detail {
	template<typename... Singletons>
	SingletonLocator GenerateSingletonLocator(TypeList<Singletons...>) {
		if constexpr (sizeof...(Singletons) > 0) {
			constexpr std::array types{ CmptAccessType::Of<Singletons>... };
			return SingletonLocator{ types.data(), types.size() };
		}
		else
			return SingletonLocator{};
	}
}

namespace Ubpa::UECS {
	template<typename Func>
	SingletonLocator SingletonLocator::Generate() {
		using ArgList = FuncTraits_ArgList<std::decay_t<Func>>;
		using SingletonList = Filter_t<ArgList, IsSingleton>;
		return detail::GenerateSingletonLocator(SingletonList{});
	}

	template<typename Func>
	SingletonLocator& SingletonLocator::Combine() {
		SingletonLocator funcLocator = Generate<Func>();
		singletonTypes = SetUnion(singletonTypes, funcLocator.singletonTypes);
		return *this;
	}
}
