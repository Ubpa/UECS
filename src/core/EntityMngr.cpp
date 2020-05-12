#include <UECS/EntityMngr.h>

using namespace Ubpa;
using namespace std;

EntityMngr::~EntityMngr() {
	for (auto p : h2a)
		delete p.second;
}

size_t EntityMngr::RequestEntityFreeEntry() {
	if (entityTableFreeEntry.empty()) {
		size_t index = entityTable.size();
		entityTable.emplace_back();
		return index;
	}

	size_t entry = entityTableFreeEntry.back();
	entityTableFreeEntry.pop_back();
	return entry;
}

void EntityMngr::RecycleEntityEntry(Entity e) {
	assert(Exist(e));

	auto& info = entityTable[e.Idx()];
	info.archetype = nullptr;
	info.idxInArchetype = static_cast<size_t>(-1);
	info.version++;

	entityTableFreeEntry.push_back(e.Idx());
}

bool EntityMngr::Exist(Entity e) {
	return e.Idx() < entityTable.size() && e.Version() == entityTable[e.Idx()].version;
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

void EntityMngr::Destroy(Entity e) {
	assert(Exist(e));
	auto info = entityTable[e.Idx()];
	auto archetype = info.archetype;
	auto idxInArchetype = info.idxInArchetype;

	auto movedIdxInArchetype = archetype->Erase(idxInArchetype);

	if (movedIdxInArchetype != Archetype::npos) {
		auto target = ai2ei.find({ archetype, movedIdxInArchetype });
		size_t movedEntityIndex = target->second;
		ai2ei.erase(target);
		ai2ei[{archetype, idxInArchetype}] = movedEntityIndex;
		entityTable[movedEntityIndex].idxInArchetype = idxInArchetype;
	}
	else
		ai2ei.erase({ archetype, idxInArchetype });

	RecycleEntityEntry(e);
}

void EntityMngr::GenJob(Job* job, SystemFunc* sys) const {
	for (Archetype* archetype : QueryArchetypes(sys->query)) {
		auto [chunkEntity, chunkCmpts, sizes] = archetype->Locate(sys->query.locator.CmptTypes());

		size_t num = archetype->EntityNum();
		size_t chunkNum = archetype->ChunkNum();
		size_t chunkCapacity = archetype->ChunkCapacity();

		for (size_t i = 0; i < chunkNum; i++) {
			size_t J = std::min(chunkCapacity, num - (i * chunkCapacity));
			job->emplace([entities = chunkEntity[i], sys, sizes = sizes, cmpts = std::move(chunkCmpts[i]), J]() mutable {
				for (size_t j = 0; j < J; j++) {
					(*sys)(entities[j], cmpts.data());
					for (size_t k = 0; k < cmpts.size(); k++) {
						reinterpret_cast<uint8_t*&>(cmpts[k]) += sizes[k];
						entities++;
					}
				}
			});
		}
	}
}
