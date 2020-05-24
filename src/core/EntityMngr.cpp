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
	info.idxInArchetype = size_t_invalid;
	info.version++;

	entityTableFreeEntry.push_back(e.Idx());
}

Archetype* EntityMngr::GetOrCreateArchetypeOf(const CmptType* types, size_t num) {
	size_t hashcode = Archetype::HashCode(types, num);
	auto target = h2a.find(hashcode);
	if (target != h2a.end())
		return target->second;

	auto archetype = Archetype::New(types, num);
	h2a[hashcode] = archetype;
	for (auto& [query, archetypes] : queryCache) {
		if (archetype->GetCmptTypeSet().IsMatch(query))
			archetypes.insert(archetype);
	}

	return archetype;
}

Entity EntityMngr::Create(const CmptType* types, size_t num) {
	Archetype* archetype = GetOrCreateArchetypeOf(types, num);
	size_t entityIndex = RequestEntityFreeEntry();
	EntityInfo& info = entityTable[entityIndex];
	Entity e{ entityIndex, info.version };
	info.archetype = archetype;
	info.idxInArchetype = archetype->Create(e);
	return e;
}

void EntityMngr::AttachWithoutInit(Entity e, const CmptType* types, size_t num) {
	assert(Exist(e));

	auto& info = entityTable[e.Idx()];
	Archetype* srcArchetype = info.archetype;
	size_t srcIdxInArchetype = info.idxInArchetype;

	const auto& srcCmptTypeSet = srcArchetype->GetCmptTypeSet();
	auto dstCmptTypeSet = srcCmptTypeSet;
	for (size_t i = 0; i < num; i++)
		dstCmptTypeSet.Insert(types[i]);
	size_t dstCmptTypeSetHashCode = dstCmptTypeSet.HashCode();

	// get dstArchetype
	Archetype* dstArchetype;
	auto target = h2a.find(dstCmptTypeSetHashCode);
	if (target == h2a.end()) {
		dstArchetype = Archetype::Add(srcArchetype, types, num);
		assert(dstCmptTypeSet == dstArchetype->GetCmptTypeSet());
		h2a[dstCmptTypeSetHashCode] = dstArchetype;
		for (auto& [query, archetypes] : queryCache) {
			if (dstCmptTypeSet.IsMatch(query))
				archetypes.insert(dstArchetype);
		}
	}
	else
		dstArchetype = target->second;

	if (dstArchetype == srcArchetype)
		return;

	// move src to dst
	size_t dstIdxInArchetype = dstArchetype->RequestBuffer();

	auto srcCmptTraits = srcArchetype->GetRTSCmptTraits();
	for (auto type : srcCmptTypeSet) {
		auto [srcCmpt, srcSize] = srcArchetype->At(type, srcIdxInArchetype);
		auto [dstCmpt, dstSize] = dstArchetype->At(type, dstIdxInArchetype);
		assert(srcSize == dstSize);
		srcCmptTraits.MoveConstruct(type, dstCmpt, srcCmpt);
	}

	// erase
	auto srcMovedEntityIndex = srcArchetype->Erase(srcIdxInArchetype);
	if (srcMovedEntityIndex != size_t_invalid)
		entityTable[srcMovedEntityIndex].idxInArchetype = srcIdxInArchetype;

	info.archetype = dstArchetype;
	info.idxInArchetype = dstIdxInArchetype;
}

void EntityMngr::Attach(Entity e, const CmptType* types, size_t num) {
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	auto srcArchetype = entityTable[e.Idx()].archetype;
	AttachWithoutInit(e, types, num);
	const auto& new_info = entityTable[e.Idx()];
	const auto& rtdct = RTDCmptTraits::Instance();
	for (size_t i = 0; i < num; i++) {
		auto type = types[i];
		if (srcArchetype->GetCmptTypeSet().IsContain(type))
			continue;
		auto target = rtdct.default_constructors.find(type);
		if (target == rtdct.default_constructors.end())
			continue;

		target->second(get<void*>(new_info.archetype->At(type, new_info.idxInArchetype)));
	}
}

void EntityMngr::Detach(Entity e, const CmptType* types, size_t num) {
	if (!Exist(e)) throw std::invalid_argument("EntityMngr::Detach: Entity is invalid");

	auto& info = entityTable[e.Idx()];
	Archetype* srcArchetype = info.archetype;
	size_t srcIdxInArchetype = info.idxInArchetype;

	const auto& srcCmptTypeSet = srcArchetype->GetCmptTypeSet();
	auto dstCmptTypeSet = srcCmptTypeSet;
	for (size_t i = 0; i < num; i++)
		dstCmptTypeSet.Erase(types[i]);
	size_t dstCmptTypeSetHashCode = dstCmptTypeSet.HashCode();

	// get dstArchetype
	Archetype* dstArchetype;
	auto target = h2a.find(dstCmptTypeSetHashCode);
	if (target == h2a.end()) {
		dstArchetype = Archetype::Remove(srcArchetype, types, num);
		assert(dstCmptTypeSet == dstArchetype->GetCmptTypeSet());
		h2a[dstCmptTypeSetHashCode] = dstArchetype;
		for (auto& [query, archetypes] : queryCache) {
			if (dstCmptTypeSet.IsMatch(query))
				archetypes.insert(dstArchetype);
		}
	}
	else
		dstArchetype = target->second;

	if (dstArchetype == srcArchetype)
		return;

	// move src to dst
	size_t dstIdxInArchetype = dstArchetype->RequestBuffer();
	auto srcCmptTraits = srcArchetype->GetRTSCmptTraits();
	for (auto type : srcCmptTypeSet) {
		auto [srcCmpt, srcSize] = srcArchetype->At(type, srcIdxInArchetype);
		if (dstCmptTypeSet.IsContain(type)) {
			auto [dstCmpt, dstSize] = dstArchetype->At(type, dstIdxInArchetype);
			assert(srcSize == dstSize);
			srcCmptTraits.MoveConstruct(type, dstCmpt, srcCmpt);
		}
		else
			srcCmptTraits.Destruct(type, srcCmpt);
	}

	// erase
	auto srcMovedEntityIndex = srcArchetype->Erase(srcIdxInArchetype);
	if (srcMovedEntityIndex != size_t_invalid)
		entityTable[srcMovedEntityIndex].idxInArchetype = srcIdxInArchetype;

	info.archetype = dstArchetype;
	info.idxInArchetype = dstIdxInArchetype;
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
					(*sys)(entities[j], indexOffsetInQueryChunk + j, { &sys->query.locator, cmpts.data() });
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
