#pragma once

#include "CmptType.h"

#include <UContainer/Span.h>

#include <set>

namespace Ubpa::UECS {
	class SingletonLocator {
	public:
		SingletonLocator(std::set<CmptAccessType> types) : singletonTypes{ std::move(types) } {}
		SingletonLocator(Span<const CmptAccessType> types);
		SingletonLocator() = default;

		template<typename Func>
		static SingletonLocator Generate();

		template<typename Func>
		SingletonLocator& Combine();

		const std::set<CmptAccessType>& SingletonTypes() const noexcept { return singletonTypes; }

		bool HasWriteSingletonType() const noexcept;

	private:
		std::set<CmptAccessType> singletonTypes;
	};
}

#include "detail/SingletonLocator.inl"
