#pragma once

#include "CmptTag.h"

#include "CmptType.h"

#include <set>

namespace Ubpa::UECS {
	class SingletonLocator {
	public:
		SingletonLocator(const CmptType* types, size_t num);

		SingletonLocator();

		template<typename Func>
		static SingletonLocator Generate();

		template<typename Func>
		SingletonLocator& Combine();

		const std::set<CmptType>& LastFrameSingletonTypes() const noexcept { return lastFrameSingletonTypes; }
		const std::set<CmptType>& WriteSingletonTypes() const noexcept { return writeSingletonTypes; }
		const std::set<CmptType>& LatestSingletonTypes() const noexcept { return latestSingletonTypes; }
		const std::set<CmptType>& SingletonTypes() const noexcept { return singletonTypes; }

	private:
		std::set<CmptType> lastFrameSingletonTypes;
		std::set<CmptType> writeSingletonTypes;
		std::set<CmptType> latestSingletonTypes;
		std::set<CmptType> singletonTypes;
	};
}

#include "detail/SingletonLocator.inl"
