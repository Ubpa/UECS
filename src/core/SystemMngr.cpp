#include <UECS/SystemMngr.h>

#include <UECS/IListener.h>

#include <cassert>

using namespace Ubpa::UECS;

SystemMngr::SystemMngr(const SystemMngr& mngr)
	:
	systems{mngr.systems},
	frees{mngr.frees},
	activeSystemIndices{mngr.activeSystemIndices}
{
	for (size_t i = 0; i < systems.size(); i++) {
		const auto& system = systems[i];

		if (system.name.empty())
			continue;

		name2idx.emplace(std::string_view{ system.name }, i);
	}
}

void SystemMngr::Clear() {
	frees.clear();
	activeSystemIndices.clear();
	name2idx.clear();
	systems.clear();
}

size_t SystemMngr::Register(std::string name, Func func) {
	assert(!name.empty());

	size_t idx;
	if (!frees.empty()) {
		idx = frees.back();
		frees.pop_back();
	}
	else {
		idx = systems.size();
		systems.emplace_back();
	}

	name2idx.emplace(std::string_view(name), idx);
	systems[idx].func = std::move(func);
	systems[idx].name = std::move(name);

	return idx;
}

void SystemMngr::Unregister(size_t idx) {
	assert(idx < systems.size());
	auto& system = systems[idx];
	if (system.name.empty())
		return;
	activeSystemIndices.erase(idx);
	name2idx.erase(system.name);
	system.func = nullptr;
	system.name.clear();
	frees.push_back(idx);
}

void SystemMngr::Unregister(std::string_view name) {
	auto target = name2idx.find(name);
	if (target == name2idx.end())
		return;

	size_t idx = target->second;

	activeSystemIndices.erase(idx);
	name2idx.erase(target);
	systems[idx].func = nullptr;
	systems[idx].name.clear();
	frees.push_back(idx);
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
