#include <UECS/detail/EntityMngr.h>
#include <UECS/Entity.h>

using namespace Ubpa;
using namespace std;

EntityMngr::~EntityMngr() {
	for (auto p : h2a)
		delete p.second;
}

const std::set<Archetype*>& EntityMngr::QueryArchetypes(const EntityQuery& query) const {
	auto target = queryCache.find(query);
	if (target != queryCache.end())
		return target->second;

	// queryCache is **mutable**
	std::set<Archetype*>& archetypes = queryCache[query];
	for (const auto& [h, a] : h2a) {
		if (a->GetCmptTypeSet().IsMatch(query))
			archetypes.insert(a);
	}

	return archetypes;
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
	else
		ai2e.erase({ archetype, idx });

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

void EntityMngr::GenJob(Job* job, SystemFunc* sys) const {
	for (Archetype* archetype : QueryArchetypes(sys->query)) {
		auto [chunkCmpts, sizes] = archetype->Locate(sys->query.Locator().CmptTypes());

		size_t num = archetype->EntityNum();
		size_t chunkNum = archetype->ChunkNum();
		size_t chunkCapacity = archetype->ChunkCapacity();

		for (size_t i = 0; i < chunkNum; i++) {
			size_t J = std::min(chunkCapacity, num - (i * chunkCapacity));
			if (sys->IsNeedEntity()) {
				job->emplace([=, sizes = sizes, cmpts = std::move(chunkCmpts[i])]() mutable {
					vector<Entity*> entities;
					entities.resize(J);
					for (size_t j = 0; j < J; j++)
						entities[j] = static_cast<Entity*>(ai2e.find({ archetype, i * chunkCapacity + j })->second);
					for (size_t j = 0; j < J; j++) {
						(*sys)(entities[j], cmpts.data());
						for (size_t k = 0; k < cmpts.size(); k++)
							reinterpret_cast<uint8_t*&>(cmpts[k]) += sizes[k];
					}
				});
			}
			else {
				job->emplace([sys, sizes = sizes, cmpts = std::move(chunkCmpts[i]), J]() mutable {
					for (size_t j = 0; j < J; j++) {
						(*sys)(nullptr, cmpts.data());
						for (size_t k = 0; k < cmpts.size(); k++)
							reinterpret_cast<uint8_t*&>(cmpts[k]) += sizes[k];
					}
				});
			}
		}
	}
}
