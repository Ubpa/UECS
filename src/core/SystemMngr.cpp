#include <UECS/SystemMngr.h>

#include <cassert>

using namespace Ubpa::UECS;

SystemMngr::SystemMngr(const SystemMngr& mngr, World* w)
	:
	systemTraits{ mngr.systemTraits },
	aliveSystemIDs{ mngr.aliveSystemIDs },
	activeSystemIDs{ mngr.activeSystemIDs },
	w{ w }
{}

SystemMngr::SystemMngr(SystemMngr&& mngr, World* w) noexcept
	:
	systemTraits{ std::move(mngr.systemTraits) },
	aliveSystemIDs{ mngr.aliveSystemIDs },
	activeSystemIDs{ mngr.activeSystemIDs },
	w{ w } 
{}

SystemMngr::~SystemMngr() {
	assert(activeSystemIDs.empty());
	assert(aliveSystemIDs.empty());
}

void SystemMngr::Clear() {
	for (auto name : activeSystemIDs)
		systemTraits.Deactivate(name, w);
	for (auto name : aliveSystemIDs)
		systemTraits.Destroy(name, w);
	activeSystemIDs.clear();
	aliveSystemIDs.clear();
}

void SystemMngr::Create(NameID ID) {
	if (IsAlive(ID))
		return;
	systemTraits.Create(ID, w);
	aliveSystemIDs.insert(ID);
}

void SystemMngr::Activate(NameID ID) {
	Create(ID);
	if (IsActive(ID))
		return;
	systemTraits.Activate(ID, w);
	activeSystemIDs.insert(ID);
}

void SystemMngr::Update(NameID ID, Schedule& schedule) const {
	assert(IsActive(ID));
	systemTraits.Update(ID, schedule);
}

void SystemMngr::Deactivate(NameID ID) {
	if (!IsAlive(ID) || !IsActive(ID))
		return;
	systemTraits.Deactivate(ID, w);
	activeSystemIDs.erase(ID);
}

void SystemMngr::Destroy(NameID ID) {
	if (!IsAlive(ID))
		return;
	Deactivate(ID);
	systemTraits.Destroy(ID, w);
	aliveSystemIDs.erase(ID);
}

bool SystemMngr::IsAlive(NameID ID) const {
	return aliveSystemIDs.find(ID) != aliveSystemIDs.end();
}

bool SystemMngr::IsActive(NameID ID) const {
	return activeSystemIDs.find(ID) != activeSystemIDs.end();
}
