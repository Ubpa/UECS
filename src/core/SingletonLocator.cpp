#include <UECS/SingletonLocator.hpp>

using namespace Ubpa::UECS;
using namespace std;

SingletonLocator::SingletonLocator(std::set<AccessTypeID> types) : singletonTypes{ std::move(types) } {}
SingletonLocator::SingletonLocator() = default;

SingletonLocator::SingletonLocator(std::span<const AccessTypeID> types) {
	for (const auto& type : types)
		singletonTypes.insert(type);
}

const std::set<AccessTypeID>& SingletonLocator::SingletonTypes() const noexcept { return singletonTypes; }

bool SingletonLocator::HasWriteSingletonType() const noexcept {
	for (const auto& type : singletonTypes) {
		if (type.GetAccessMode() == AccessMode::WRITE)
			return true;
	}
	return false;
}
