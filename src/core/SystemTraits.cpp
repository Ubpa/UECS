#include <cassert>
#include <UECS/SystemTraits.h>

using namespace Ubpa;
using namespace Ubpa::UECS;

SystemTraits::SystemTraits(const SystemTraits& traits) :
	createMap{ traits.createMap },
	activateMap{ traits.activateMap },
	updateMap{ traits.updateMap },
	deactivateMap{ traits.deactivateMap },
	destroyMap{ traits.destroyMap }
{
	for (const auto& [id, name] : traits.names) {
		char* buffer = (char*)rsrc.allocate((name.size() + 1) * sizeof(char), alignof(char));
		std::memcpy(buffer, name.data(), name.size() * sizeof(char));
		buffer[name.size()] = 0;
		names.emplace(id, std::string_view{ buffer, name.size() });
		buffer += name.size() + 1;
	}
}

SystemTraits& SystemTraits::operator=(const SystemTraits& rhs) {
	createMap = rhs.createMap;
	activateMap = rhs.activateMap;
	updateMap = rhs.updateMap;
	deactivateMap = rhs.deactivateMap;
	destroyMap = rhs.destroyMap;
	rsrc.release();
	for (const auto& [id, name] : rhs.names) {
		char* buffer = (char*)rsrc.allocate((name.size() + 1) * sizeof(char), alignof(char));
		std::memcpy(buffer, name.data(), name.size() * sizeof(char));
		buffer[name.size()] = 0;
		names.emplace(id, std::string_view{ buffer, name.size() });
		buffer += name.size() + 1;
	}
	return *this;
}

Name SystemTraits::Register(std::string_view name) {
	assert(!name.empty());
	
	NameID id{ name };
	auto target = names.find(id);
	if (target != names.end())
		return { target->second, id };

	auto* buffer = (char*)rsrc.allocate((name.size() + 1) * sizeof(char), alignof(char));
	std::memcpy(buffer, name.data(), name.size() * sizeof(char));
	buffer[name.size()] = 0;
	std::string_view newname{ buffer, name.size() };
	names.emplace(id, newname);

	return { newname, id };
}

void SystemTraits::Unregister(NameID ID) {
	auto target = names.find(ID);
	if (target == names.end())
		return;

	names.erase(target);
	createMap.erase(ID);
	activateMap.erase(ID);
	updateMap.erase(ID);
	deactivateMap.erase(ID);
	destroyMap.erase(ID);
}

bool SystemTraits::IsRegistered(NameID ID) const noexcept {
	return names.contains(ID);
}

void SystemTraits::RegisterOnCreate(NameID ID, std::function<OnCreate> func) {
	assert(IsRegistered(ID));
	createMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnActivate(NameID ID, std::function<OnActivate> func) {
	assert(IsRegistered(ID));
	activateMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnUpdate(NameID ID, std::function<OnUpdate> func) {
	assert(IsRegistered(ID));
	updateMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnDeactivate(NameID ID, std::function<OnDeactivate> func) {
	assert(IsRegistered(ID));
	deactivateMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnDestroy(NameID ID, std::function<OnDestroy> func) {
	assert(IsRegistered(ID));
	destroyMap[ID] = std::move(func);
}

std::string_view SystemTraits::Nameof(NameID ID) const noexcept {
	auto target = names.find(ID);
	if (target == names.end())
		return {};
	return target->second;
}

void SystemTraits::Create(NameID ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = createMap.find(ID);
	if (target != createMap.end())
		target->second(w);
}

void SystemTraits::Activate(NameID ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = activateMap.find(ID);
	if (target != activateMap.end())
		target->second(w);
}

void SystemTraits::Update(NameID ID, Schedule& schedule) const {
	assert(IsRegistered(ID));
	auto target = updateMap.find(ID);
	if (target != updateMap.end())
		target->second(schedule);
}

void SystemTraits::Deactivate(NameID ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = deactivateMap.find(ID);
	if (target != deactivateMap.end())
		target->second(w);
}

void SystemTraits::Destroy(NameID ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = destroyMap.find(ID);
	if (target != destroyMap.end())
		target->second(w);
}
