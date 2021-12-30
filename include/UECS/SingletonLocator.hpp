#pragma once

#include "AccessTypeID.hpp"

#include <set>
#include <span>

namespace Ubpa::UECS {
	class SingletonLocator {
	public:
		SingletonLocator(std::set<AccessTypeID> types);
		SingletonLocator(std::span<const AccessTypeID> types);
		SingletonLocator();

		template<typename Func>
		static SingletonLocator Generate();

		template<typename Func>
		SingletonLocator& Combine();

		const std::set<AccessTypeID>& SingletonTypes() const noexcept;

		bool HasWriteSingletonType() const noexcept;

	private:
		std::set<AccessTypeID> singletonTypes;
	};
}

#include "details/SingletonLocator.inl"
