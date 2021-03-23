#include <cassert>
#include <UECS/SystemTraits.hpp>

using namespace Ubpa;
using namespace Ubpa::UECS;

struct SystemTraits::Impl {
	Impl(std::pmr::unsynchronized_pool_resource* r) :
		rsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>(r) },
		names{ rsrc.get() },
		createMap{ rsrc.get() },
		activateMap{ rsrc.get() },
		deactivateMap{ rsrc.get() },
		destroyMap{ rsrc.get() } {}

	Impl(const Impl& other, std::pmr::unsynchronized_pool_resource* r) :
		rsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>(r) },
		createMap{ other.createMap, rsrc.get() },
		activateMap{ other.activateMap, rsrc.get() },
		updateMap{ other.updateMap, rsrc.get() },
		deactivateMap{ other.deactivateMap, rsrc.get() },
		destroyMap{ other.destroyMap, rsrc.get() }
	{
		for (const auto& [id, name] : other.names) {
			char* buffer = (char*)rsrc->allocate((name.size() + 1) * sizeof(char), alignof(char));
			std::memcpy(buffer, name.data(), name.size() * sizeof(char));
			buffer[name.size()] = 0;
			names.emplace(id, std::string_view{ buffer, name.size() });
			buffer += name.size() + 1;
		}
	}

	std::unique_ptr<std::pmr::unsynchronized_pool_resource> rsrc;

	std::pmr::unordered_map<NameID, std::string_view> names;

	std::pmr::unordered_map<NameID, std::function<OnCreate    >> createMap;
	std::pmr::unordered_map<NameID, std::function<OnActivate  >> activateMap;
	std::pmr::unordered_map<NameID, std::function<OnUpdate    >> updateMap;
	std::pmr::unordered_map<NameID, std::function<OnDeactivate>> deactivateMap;
	std::pmr::unordered_map<NameID, std::function<OnDestroy   >> destroyMap;
};

SystemTraits::SystemTraits(std::pmr::unsynchronized_pool_resource* r) :
	impl{std::make_unique<Impl>(r)} {}

SystemTraits::SystemTraits(const SystemTraits& other, std::pmr::unsynchronized_pool_resource* r) :
	impl{ std::make_unique<Impl>(*other.impl, r) } {}

SystemTraits::SystemTraits(SystemTraits&&) noexcept = default;

SystemTraits::~SystemTraits() = default;

Name SystemTraits::Register(std::string_view name) {
	assert(!name.empty());
	
	NameID id{ name };
	auto target = impl->names.find(id);
	if (target != impl->names.end())
		return { target->second, id };

	auto* buffer = (char*)impl->rsrc->allocate((name.size() + 1) * sizeof(char), alignof(char));
	std::memcpy(buffer, name.data(), name.size() * sizeof(char));
	buffer[name.size()] = 0;
	std::string_view newname{ buffer, name.size() };
	impl->names.emplace(id, newname);

	return { newname, id };
}

void SystemTraits::Unregister(NameID ID) {
	auto target = impl->names.find(ID);
	if (target == impl->names.end())
		return;

	impl->names.erase(target);
	impl->createMap.erase(ID);
	impl->activateMap.erase(ID);
	impl->updateMap.erase(ID);
	impl->deactivateMap.erase(ID);
	impl->destroyMap.erase(ID);
}

bool SystemTraits::IsRegistered(NameID ID) const noexcept {
	return impl->names.contains(ID);
}

const std::pmr::unordered_map<NameID, std::string_view>& SystemTraits::GetNames() const noexcept {
	return impl->names;
}

void SystemTraits::RegisterOnCreate(NameID ID, std::function<OnCreate> func) {
	assert(IsRegistered(ID));
	impl->createMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnActivate(NameID ID, std::function<OnActivate> func) {
	assert(IsRegistered(ID));
	impl->activateMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnUpdate(NameID ID, std::function<OnUpdate> func) {
	assert(IsRegistered(ID));
	impl->updateMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnDeactivate(NameID ID, std::function<OnDeactivate> func) {
	assert(IsRegistered(ID));
	impl->deactivateMap[ID] = std::move(func);
}

void SystemTraits::RegisterOnDestroy(NameID ID, std::function<OnDestroy> func) {
	assert(IsRegistered(ID));
	impl->destroyMap[ID] = std::move(func);
}

std::string_view SystemTraits::Nameof(NameID ID) const noexcept {
	auto target = impl->names.find(ID);
	if (target == impl->names.end())
		return {};
	return target->second;
}

void SystemTraits::Create(NameID ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = impl->createMap.find(ID);
	if (target != impl->createMap.end())
		target->second(w);
}

void SystemTraits::Activate(NameID ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = impl->activateMap.find(ID);
	if (target != impl->activateMap.end())
		target->second(w);
}

void SystemTraits::Update(NameID ID, Schedule& schedule) const {
	assert(IsRegistered(ID));
	auto target = impl->updateMap.find(ID);
	if (target != impl->updateMap.end())
		target->second(schedule);
}

void SystemTraits::Deactivate(NameID ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = impl->deactivateMap.find(ID);
	if (target != impl->deactivateMap.end())
		target->second(w);
}

void SystemTraits::Destroy(NameID ID, World* w) const {
	assert(IsRegistered(ID));
	auto target = impl->destroyMap.find(ID);
	if (target != impl->destroyMap.end())
		target->second(w);
}
