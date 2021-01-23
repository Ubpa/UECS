#include <cassert>
#include <UECS/SystemTraits.h>

using namespace Ubpa::UECS;

SystemTraits::SystemTraits(const SystemTraits& traits)
	:
	names{ traits.names },
	createMap{ traits.createMap },
	activateMap{ traits.activateMap },
	updateMap{ traits.updateMap },
	deactivateMap{ traits.deactivateMap },
	destroyMap{ traits.destroyMap }
{
	for (std::size_t i = 0; i < names.size(); i++) {
		const auto& name = names[i];
		name2id.emplace(std::string_view{ name }, i);
	}
}

SystemTraits& SystemTraits::operator=(SystemTraits& rhs) {
	names = rhs.names;
	createMap = rhs.createMap;
	activateMap = rhs.activateMap;
	updateMap = rhs.updateMap;
	deactivateMap = rhs.deactivateMap;
	destroyMap = rhs.destroyMap;
	for (std::size_t i = 0; i < names.size(); i++) {
		const auto& name = names[i];
		name2id.emplace(std::string_view{ name }, i);
	}
	return *this;
}

std::size_t SystemTraits::Register(std::string name) {
	assert(!name.empty());
	
	auto target = name2id.find(name);
	if (target != name2id.end())
		return target->second;

	std::size_t idx = names.size();
	names.push_back(std::move(name));
	name2id.emplace_hint(target, std::pair{ std::string_view{names[idx]}, idx });

	return idx;
}

bool SystemTraits::IsRegistered(std::size_t ID) const noexcept {
	return ID < names.size();
}

std::size_t SystemTraits::GetID(std::string_view name) const {
	auto target = name2id.find(name);
	return target == name2id.end() ? static_cast<std::size_t>(-1) : target->second;
}

void SystemTraits::RegisterOnCreate(std::size_t ID, std::function<OnCreate> func) {
	assert(IsRegistered(ID));
	createMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnActivate(std::size_t ID, std::function<OnActivate> func) {
	assert(IsRegistered(ID));
	activateMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnUpdate(std::size_t ID, std::function<OnUpdate> func) {
	assert(IsRegistered(ID));
	updateMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnDeactivate(std::size_t ID, std::function<OnDeactivate> func) {
	assert(IsRegistered(ID));
	deactivateMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnDestroy(std::size_t ID, std::function<OnDestroy> func) {
	assert(IsRegistered(ID));
	destroyMap[ID] = std::move(func);
}

std::string_view SystemTraits::Nameof(std::size_t ID) const noexcept {
	assert(ID < names.size());
	return names[ID];
}

void SystemTraits::Create(std::size_t ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = createMap.find(ID);
	if (target != createMap.end())
		target->second(w);
}

void SystemTraits::Activate(std::size_t ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = activateMap.find(ID);
	if (target != activateMap.end())
		target->second(w);
}

void SystemTraits::Update(std::size_t ID, Schedule& schedule) const {
	assert(IsRegistered(ID));
	auto target = updateMap.find(ID);
	if (target != updateMap.end())
		target->second(schedule);
}

void SystemTraits::Deactivate(std::size_t ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = deactivateMap.find(ID);
	if (target != deactivateMap.end())
		target->second(w);
}

void SystemTraits::Destroy(std::size_t ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = destroyMap.find(ID);
	if (target != destroyMap.end())
		target->second(w);
}
