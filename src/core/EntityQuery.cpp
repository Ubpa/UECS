#include <UECS/EntityQuery.hpp>

using namespace Ubpa::UECS;

std::size_t EntityQuery::GetValue() const noexcept { return hash_combine(filter.GetValue(), locator.GetValue()); }

bool EntityQuery::IsMatch(const small_flat_set<TypeID>& types) const noexcept {
	return std::find_if_not(filter.all.begin(), filter.all.end(),
		[&](const auto& type) { return types.contains(type); }) == filter.all.end()
		&& (filter.any.empty()
			|| std::find_if(filter.any.begin(), filter.any.end(),
				[&](const auto& type) { return types.contains(type); }) != filter.any.end())
		&& std::find_if(filter.none.begin(), filter.none.end(),
			[&](const auto& type) { return types.contains(type); }) == filter.none.end()
		&& std::find_if_not(locator.AccessTypeIDs().begin(), locator.AccessTypeIDs().end(),
			[&](const auto& type) { return types.contains(type); }) == locator.AccessTypeIDs().end();
}

namespace Ubpa::UECS {
	bool operator==(const EntityQuery& lhs, const EntityQuery& rhs) noexcept {
		return lhs.filter == rhs.filter && lhs.locator == rhs.locator;
	}
}