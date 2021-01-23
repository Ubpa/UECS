#pragma once

#include "AccessTypeID.h"

#include <set>
#include <span>

namespace Ubpa::UECS {
	class SingletonLocator {
	public:
		SingletonLocator(std::set<AccessTypeID> types) : singletonTypes{ std::move(types) } {}
		SingletonLocator(std::span<const AccessTypeID> types);
		SingletonLocator() = default;

		template<typename Func>
		static SingletonLocator Generate();

		template<typename Func>
		SingletonLocator& Combine();

		const std::set<AccessTypeID>& SingletonTypes() const noexcept { return singletonTypes; }

		bool HasWriteSingletonType() const noexcept;

	private:
		std::set<AccessTypeID> singletonTypes;
	};
}

#include "details/SingletonLocator.inl"
