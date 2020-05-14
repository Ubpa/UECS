#include <UECS/EntityMngr.h>

using namespace Ubpa;
using namespace std;

EntityMngr::~EntityMngr() {
	for (auto [h, a] : h2a)
		delete a;
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

vector<CmptPtr> EntityMngr::Components(Entity e) const {
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	const auto& info = entityTable[e.Idx()];
	return info.archetype->Components(info.idxInArchetype);
}

bool EntityMngr::Exist(Entity e) const {
	return e.Idx() < entityTable.size() && e.Version() == entityTable[e.Idx()].version;
}

Entity EntityMngr::Instantiate(Entity srcEntity) {
	if (!Exist(srcEntity)) throw std::invalid_argument("Entity is invalid");

	size_t dstEntityIndex = RequestEntityFreeEntry();
	const auto& srcInfo = entityTable[srcEntity.Idx()];
	auto& dstInfo = entityTable[dstEntityIndex];
	Entity dstEntity{ dstEntityIndex, dstInfo.version };
	size_t dstIndexInArchetype = srcInfo.archetype->Instantiate(dstEntity, srcInfo.idxInArchetype);
	dstInfo.archetype = srcInfo.archetype;
	dstInfo.idxInArchetype = dstIndexInArchetype;
	return dstEntity;
}

const set<Archetype*>& EntityMngr::QueryArchetypes(const EntityQuery& query) const {
	auto target = queryCache.find(query);
	if (target != queryCache.end())
		return target->second;

	// queryCache is **mutable**
	set<Archetype*>& archetypes = queryCache[query];
	for (const auto& [h, a] : h2a) {
		if (a->GetCmptTypeSet().IsMatch(query))
			archetypes.insert(a);
	}

	return archetypes;
}

size_t EntityMngr::EntityNum(const EntityQuery& query) const {
	size_t sum = 0;
	for (const auto& archetype : QueryArchetypes(query))
		sum += archetype->EntityNum();
	return sum;
}

void EntityMngr::Destroy(Entity e) {
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	auto info = entityTable[e.Idx()];
	auto archetype = info.archetype;
	auto idxInArchetype = info.idxInArchetype;

	auto movedEntityIndex = archetype->Erase(idxInArchetype);

	if (movedEntityIndex != size_t_invalid)
		entityTable[movedEntityIndex].idxInArchetype = idxInArchetype;

	RecycleEntityEntry(e);
}

void EntityMngr::GenJob(Job* job, SystemFunc* sys) const {
	size_t indexOffsetInQuery = 0;
	for (Archetype* archetype : QueryArchetypes(sys->query)) {
		auto [chunkEntity, chunkCmpts, sizes] = archetype->Locate(sys->query.locator.CmptTypes());

		size_t num = archetype->EntityNum();
		size_t chunkNum = archetype->ChunkNum();
		size_t chunkCapacity = archetype->ChunkCapacity();

		for (size_t i = 0; i < chunkNum; i++) {
			job->emplace([=, sizes = sizes, entities = chunkEntity[i], cmpts = move(chunkCmpts[i])]() mutable {
				size_t idxOffsetInChunk = i * chunkCapacity;
				size_t indexOffsetInQueryChunk = indexOffsetInQuery + idxOffsetInChunk;

				size_t J = min(chunkCapacity, num - idxOffsetInChunk);
				for (size_t j = 0; j < J; j++) {
					(*sys)(entities[j], indexOffsetInQueryChunk + j, cmpts.data());
					for (size_t k = 0; k < cmpts.size(); k++)
						reinterpret_cast<uint8_t*&>(cmpts[k]) += sizes[k];
				}
			});
		}

		indexOffsetInQuery += num;
	}
}

void EntityMngr::AddCommand(const function<void()>& command) {
	lock_guard<mutex> guard(commandBufferMutex);
	commandBuffer.push_back(command);
}

void EntityMngr::RunCommands() {
	lock_guard<mutex> guard(commandBufferMutex);
	for (const auto& command : commandBuffer)
		command();
	commandBuffer.clear();
}
