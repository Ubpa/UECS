#include <UECS/SystemMngr.hpp>

#include <UECS/World.hpp>

#include <cassert>

using namespace Ubpa::UECS;

struct SystemMngr::Impl {
	Impl(World* w) :
		aliveSystemIDs{ w->GetUnsyncResource() },
		activeSystemIDs{ w->GetUnsyncResource() } {}
	Impl(const Impl& other, World* w) :
		aliveSystemIDs{ other.activeSystemIDs, w->GetUnsyncResource() },
		activeSystemIDs{ other.activeSystemIDs, w->GetUnsyncResource() } {}

	std::pmr::unordered_set<NameID> aliveSystemIDs;
	std::pmr::unordered_set<NameID> activeSystemIDs;
};

SystemMngr::SystemMngr(World* w) :
	w{ w },
	impl{ std::make_unique<Impl>(w)},
	systemTraits{ w->GetUnsyncResource() } {}

SystemMngr::SystemMngr(const SystemMngr& mngr, World* w) :
	w{ w },
	impl{ std::make_unique<Impl>(*mngr.impl, w) },
	systemTraits{ mngr.systemTraits, w->GetUnsyncResource() } {}

SystemMngr::SystemMngr(SystemMngr&& mngr, World* w) noexcept :
	w{ w },
	impl{ std::move(mngr.impl) },
	systemTraits{ std::move(mngr.systemTraits) } {}

SystemMngr::~SystemMngr() {
	Clear();
}

const std::pmr::unordered_set<Ubpa::NameID>& SystemMngr::GetAliveSystemIDs() const noexcept { return impl->aliveSystemIDs; }
const std::pmr::unordered_set<Ubpa::NameID>& SystemMngr::GetActiveSystemIDs() const noexcept { return impl->activeSystemIDs; }

void SystemMngr::Clear() {
	if (!impl)
		return;

	for (auto name : impl->activeSystemIDs)
		systemTraits.Deactivate(name, w);
	for (auto name : impl->aliveSystemIDs)
		systemTraits.Destroy(name, w);
	impl->activeSystemIDs.clear();
	impl->aliveSystemIDs.clear();
}

void SystemMngr::Create(NameID ID) {
	if (IsAlive(ID))
		return;
	systemTraits.Create(ID, w);
	impl->aliveSystemIDs.insert(ID);
}

void SystemMngr::Activate(NameID ID) {
	Create(ID);
	if (IsActive(ID))
		return;
	systemTraits.Activate(ID, w);
	impl->activeSystemIDs.insert(ID);
}

void SystemMngr::Update(NameID ID, Schedule& schedule) const {
	assert(IsActive(ID));
	systemTraits.Update(ID, schedule);
}

void SystemMngr::Deactivate(NameID ID) {
	if (!IsAlive(ID) || !IsActive(ID))
		return;
	systemTraits.Deactivate(ID, w);
	impl->activeSystemIDs.erase(ID);
}

void SystemMngr::Destroy(NameID ID) {
	if (!IsAlive(ID))
		return;
	Deactivate(ID);
	systemTraits.Destroy(ID, w);
	impl->aliveSystemIDs.erase(ID);
}

bool SystemMngr::IsAlive(NameID ID) const {
	return impl->aliveSystemIDs.contains(ID);
}

bool SystemMngr::IsActive(NameID ID) const {
	return impl->activeSystemIDs.contains(ID);
}
