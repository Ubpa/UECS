#include <UECS/detail/EntityMngr.h>

using namespace Ubpa;
using namespace std;

EntityMngr::~EntityMngr() {
	for (auto p : h2a)
		delete p.second;
}

void EntityMngr::Release(EntityData* e) {
	auto archetype = e->archetype;
	auto idx = e->idx;
	entityPool.Recycle(e);

	auto movedEntityIdx = archetype->Erase(idx);

	if (movedEntityIdx != static_cast<size_t>(-1)) {
		auto target = ai2e.find({ archetype, movedEntityIdx });
		EntityData* movedEntity = target->second;
		ai2e.erase(target);
		movedEntity->idx = idx;
		ai2e[{archetype, idx}] = movedEntity;
	}

	/*if (archetype->Size() == 0 && archetype->CmptNum() != 0) {
		h2a.erase(archetype->cmptTypeSet);
		delete archetype;
	}*/

	archetype = nullptr;
	idx = static_cast<size_t>(-1);
}

void EntityMngr::AddCommand(const std::function<void()>& command) {
	lock_guard<mutex> guard(commandBufferMutex);
	commandBuffer.push_back(command);
}

void EntityMngr::RunCommands() {
	lock_guard<mutex> guard(commandBufferMutex);
	for (const auto& command : commandBuffer)
		command();
	commandBuffer.clear();
}
