#include <UECS/SystemMngr.h>

#include <UECS/IListener.h>

#include <cassert>

using namespace Ubpa::UECS;

void SystemMngr::Clear() {
	systems.clear();
	name2idx.clear();
	activeSystemIndices.clear();
}

size_t SystemMngr::Register(std::string name, void(*func)(Schedule&)) {
	size_t idx = systems.size();
	systems.emplace_back(func);
	name2idx.emplace(std::move(name), idx);
	return idx;
}

void SystemMngr::Activate(size_t index) {
	assert(index < systems.size());
	activeSystemIndices.insert(index);
}

void SystemMngr::Deactivate(size_t index) {
	assert(index < systems.size());
	activeSystemIndices.erase(index);
}

size_t SystemMngr::GetIndex(std::string_view name) const {
	auto target = name2idx.find(name);
	return target != name2idx.end() ? target->second : static_cast<size_t>(-1);
}
