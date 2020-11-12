#include <UECS/EntityMngr.h>

#include <UECS/detail/SystemFunc.h>
#include <UECS/IListener.h>

using namespace Ubpa::UECS;
using namespace std;

EntityMngr::EntityMngr()
	: sharedChunkPool{ std::make_unique<Pool<Chunk>>() }
{}

EntityMngr::EntityMngr(const EntityMngr& em)
	:
	cmptTraits{ em.cmptTraits },
	sharedChunkPool{ std::make_unique<Pool<Chunk>>() }
{
	ts2a.reserve(em.ts2a.size());
	for (const auto& [ts, a] : em.ts2a) {
		auto [iter, success] = ts2a.try_emplace(ts, std::make_unique<Archetype>(sharedChunkPool.get(), *a));
		assert(success);
	}
	entityTableFreeEntry = em.entityTableFreeEntry;
	entityTable.resize(em.entityTable.size());
	for (size_t i = 0; i < em.entityTable.size(); i++) {
		auto& dst = entityTable[i];
		const auto& src = em.entityTable[i];
		dst.idxInArchetype = src.idxInArchetype;
		dst.version = src.version;
		if (src.archetype)
			dst.archetype = ts2a.at(src.archetype->types).get();
	}
	queryCache.reserve(em.queryCache.size());
	for (const auto& [query, srcArchetypes] : em.queryCache) {
		auto& dstArchetypes = queryCache[query];
		for (auto* archetype : srcArchetypes)
			dstArchetypes.insert(ts2a.at(archetype->types).get());
	}
}

EntityMngr::~EntityMngr() {
	ts2a.clear();
	sharedChunkPool->FastClear();
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

Archetype* EntityMngr::GetOrCreateArchetypeOf(Span<const CmptType> types) {
	auto typeset = Archetype::GenCmptTypeSet(types);
	auto target = ts2a.find(typeset);
	if (target != ts2a.end())
		return target->second.get();

	auto* archetype = Archetype::New(cmptTraits, sharedChunkPool.get(), types);

	ts2a.emplace(std::move(typeset), std::unique_ptr<Archetype>{ archetype });
	for (auto& [query, archetypes] : queryCache) {
		if (archetype->GetCmptTypeSet().IsMatch(query))
			archetypes.insert(archetype);
	}

	return archetype;
}

Entity EntityMngr::Create(Span<const CmptType> types) {
	Archetype* archetype = GetOrCreateArchetypeOf(types);
	size_t entityIndex = RequestEntityFreeEntry();
	auto& info = entityTable[entityIndex];
	Entity e{ entityIndex, info.version };
	info.archetype = archetype;
	info.idxInArchetype = archetype->Create(cmptTraits, e);
	return e;
}

Archetype* EntityMngr::AttachWithoutInit(Entity e, Span<const CmptType> types) {
	assert(IsSet(types));
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	auto& info = entityTable[e.Idx()];
	Archetype* srcArchetype = info.archetype;
	size_t srcIdxInArchetype = info.idxInArchetype;

	const auto& srcCmptTypeSet = srcArchetype->GetCmptTypeSet();
	auto dstCmptTypeSet = srcCmptTypeSet;
	dstCmptTypeSet.Insert(types);

	// get dstArchetype
	Archetype* dstArchetype;
	auto target = ts2a.find(dstCmptTypeSet);
	if (target == ts2a.end()) {
		dstArchetype = Archetype::Add(cmptTraits, srcArchetype, types);
		assert(dstCmptTypeSet == dstArchetype->GetCmptTypeSet());
		for (auto& [query, archetypes] : queryCache) {
			if (dstCmptTypeSet.IsMatch(query))
				archetypes.insert(dstArchetype);
		}
		ts2a.emplace(std::move(dstCmptTypeSet), std::unique_ptr<Archetype>{dstArchetype});
	}
	else {
		dstArchetype = target->second.get();
		if (dstArchetype == srcArchetype)
			return srcArchetype;
	}

	// move src to dst
	size_t dstIdxInArchetype = dstArchetype->RequestBuffer();

	const auto& srcCmptTraits = srcArchetype->GetArchetypeCmptTraits();
	for (const auto& type : srcCmptTypeSet.data) {
		auto* srcCmpt = srcArchetype->At(type, srcIdxInArchetype);
		auto* dstCmpt = dstArchetype->At(type, dstIdxInArchetype);
		srcCmptTraits.MoveConstruct(type, dstCmpt, srcCmpt);
	}

	// erase
	auto srcMovedEntityIndex = srcArchetype->Erase(srcIdxInArchetype);
	if (srcMovedEntityIndex != static_cast<size_t>(-1))
		entityTable[srcMovedEntityIndex].idxInArchetype = srcIdxInArchetype;

	info.archetype = dstArchetype;
	info.idxInArchetype = dstIdxInArchetype;

	return srcArchetype;
}

void EntityMngr::Attach(Entity e, Span<const CmptType> types) {
	auto* origArchetype = AttachWithoutInit(e, types);
	const auto& info = entityTable[e.Idx()];
	for (const auto& type : types) {
		if (origArchetype->GetCmptTypeSet().Contains(type))
			continue;

		auto target = cmptTraits.GetDefaultConstructors().find(type);
		if (target == cmptTraits.GetDefaultConstructors().end())
			continue;

		target->second(info.archetype->At(type, info.idxInArchetype));
	}
}

void EntityMngr::Detach(Entity e, Span<const CmptType> types) {
	assert(IsSet(types));
	if (!Exist(e)) throw std::invalid_argument("EntityMngr::Detach: Entity is invalid");

	auto& info = entityTable[e.Idx()];
	Archetype* srcArchetype = info.archetype;

	const auto& srcCmptTypeSet = srcArchetype->GetCmptTypeSet();
	auto dstCmptTypeSet = srcCmptTypeSet;
	dstCmptTypeSet.Erase(types);

	// get dstArchetype
	Archetype* dstArchetype;
	bool isNewArchetype;
	auto target = ts2a.find(dstCmptTypeSet);
	if (target == ts2a.end()) {
		isNewArchetype = true;
		dstArchetype = Archetype::Remove(srcArchetype, types);
		assert(dstCmptTypeSet == dstArchetype->GetCmptTypeSet());
		for (auto& [query, archetypes] : queryCache) {
			if (dstCmptTypeSet.IsMatch(query))
				archetypes.insert(dstArchetype);
		}
	}
	else {
		isNewArchetype = false;
		dstArchetype = target->second.get();
		if (dstArchetype == srcArchetype)
			return;
	}

	// move src to dst
	size_t srcIdxInArchetype = info.idxInArchetype;
	size_t dstIdxInArchetype = dstArchetype->RequestBuffer();

	const auto& srcCmptTraits = srcArchetype->GetArchetypeCmptTraits();
	for (const auto& type : dstCmptTypeSet.data) {
		auto* srcCmpt = srcArchetype->At(type, srcIdxInArchetype);
		auto* dstCmpt = dstArchetype->At(type, dstIdxInArchetype);
		srcCmptTraits.MoveConstruct(type, dstCmpt, srcCmpt);
	}

	// erase
	size_t srcMovedEntityIndex = srcArchetype->Erase(srcIdxInArchetype); // call destructor
	if (srcMovedEntityIndex != static_cast<size_t>(-1))
		entityTable[srcMovedEntityIndex].idxInArchetype = srcIdxInArchetype;

	info.archetype = dstArchetype;
	info.idxInArchetype = dstIdxInArchetype;

	if(isNewArchetype)
		ts2a.emplace(std::move(dstCmptTypeSet), std::unique_ptr<Archetype>{dstArchetype});
}

vector<CmptPtr> EntityMngr::Components(Entity e) const {
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	const auto& info = entityTable[e.Idx()];
	return info.archetype->Components(info.idxInArchetype);
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

bool EntityMngr::IsSet(Span<const CmptType> types) noexcept {
	for (size_t i = 0; i < types.size(); i++) {
		for (size_t j = 0; j < i; j++)
			if (types[i] == types[j])
				return false;
	}

	return true;
}

const set<Archetype*>& EntityMngr::QueryArchetypes(const EntityQuery& query) const {
	auto target = queryCache.find(query);
	if (target != queryCache.end())
		return target->second;

	// queryCache is **mutable**
	set<Archetype*>& archetypes = queryCache[query];
	for (const auto& [ts, a] : ts2a) {
		if (a->GetCmptTypeSet().IsMatch(query))
			archetypes.insert(a.get());
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
	auto* archetype = info.archetype;
	auto idxInArchetype = info.idxInArchetype;

	auto movedEntityIndex = archetype->Erase(idxInArchetype);

	if (movedEntityIndex != static_cast<size_t>(-1))
		entityTable[movedEntityIndex].idxInArchetype = idxInArchetype;

	RecycleEntityEntry(e);
}

tuple<bool, vector<CmptAccessPtr>> EntityMngr::LocateSingletons(const SingletonLocator& locator) const {
	size_t numSingletons = 0;
	vector<CmptAccessPtr> rst;
	rst.reserve(locator.SingletonTypes().size());
	for (const auto& t : locator.SingletonTypes()) {
		auto ptr = GetSingleton(t);
		if (ptr.Ptr() == nullptr)
			return { false, {} };
		rst.emplace_back(ptr, t.GetAccessMode());
	}
	return { true, rst };
}

void EntityMngr::GenEntityJob(World* w, Job* job, SystemFunc* sys) const {
	assert(sys->GetMode() == SystemFunc::Mode::Entity);

	auto [success, singletons] = LocateSingletons(sys->singletonLocator);
	if (!success)
		return;
	
	if (sys->IsParallel()) {
		assert(job);
		size_t indexOffsetInQuery = 0;
		for (Archetype* archetype : QueryArchetypes(sys->entityQuery)) {
			auto [chunkEntity, chunkCmpts, sizes] = archetype->Locate(sys->entityQuery.locator);

			size_t num = archetype->EntityNum();
			size_t chunkNum = archetype->ChunkNum();
			size_t chunkCapacity = archetype->ChunkCapacity();

			for (size_t i = 0; i < chunkNum; i++) {
				job->emplace([=, sizes = sizes, entities = chunkEntity[i], cmpts = move(chunkCmpts[i]), singletons = singletons]() mutable {
					size_t idxOffsetInChunk = i * chunkCapacity;
					size_t indexOffsetInQueryChunk = indexOffsetInQuery + idxOffsetInChunk;
					CmptsView chunkView{ Span{cmpts.data(), cmpts.size()} };
					SingletonsView singletonsView{ Span{singletons.data(), singletons.size()} };

					size_t J = min(chunkCapacity, num - idxOffsetInChunk);
					for (size_t j = 0; j < J; j++) {
						(*sys)(
							w,
							singletonsView,
							entities[j],
							indexOffsetInQueryChunk + j,
							chunkView
						);
						for (size_t k = 0; k < cmpts.size(); k++)
							reinterpret_cast<uint8_t*&>(cmpts[k].p) += sizes[k];
					}
				});
			}

			indexOffsetInQuery += num;
		}
	}
	else {
		auto work = [this, singletons = std::move(singletons), sys, w]() {
			size_t indexOffsetInQuery = 0;
			for (Archetype* archetype : QueryArchetypes(sys->entityQuery)) {
				auto [chunkEntity, chunkCmpts, sizes] = archetype->Locate(sys->entityQuery.locator);

				size_t num = archetype->EntityNum();
				size_t chunkNum = archetype->ChunkNum();
				size_t chunkCapacity = archetype->ChunkCapacity();

				for (size_t i = 0; i < chunkNum; i++) {
					size_t idxOffsetInChunk = i * chunkCapacity;
					size_t indexOffsetInQueryChunk = indexOffsetInQuery + idxOffsetInChunk;
					CmptsView chunkView{ Span{chunkCmpts[i].data(), chunkCmpts[i].size()} };
					SingletonsView singletonsView{ Span{singletons.data(), singletons.size()} };

					size_t J = min(chunkCapacity, num - idxOffsetInChunk);
					for (size_t j = 0; j < J; j++) {
						(*sys)(
							w,
							singletonsView,
							chunkEntity[i][j],
							indexOffsetInQueryChunk + j,
							chunkView
							);
						for (size_t k = 0; k < chunkCmpts[i].size(); k++)
							reinterpret_cast<uint8_t*&>(chunkCmpts[i][k].p) += sizes[k];
					}
				}

				indexOffsetInQuery += num;
			}
		};

		if (job)
			job->emplace(std::move(work));
		else
			work();
	}
}

void EntityMngr::GenChunkJob(World* w, Job* job, SystemFunc* sys) const {
	assert(sys->GetMode() == SystemFunc::Mode::Chunk);

	auto [success, singletons] = LocateSingletons(sys->singletonLocator);
	if (!success)
		return;

	if (sys->IsParallel()) {
		assert(job != nullptr);
		size_t indexOffsetInQuery = 0;
		for (Archetype* archetype : QueryArchetypes(sys->entityQuery)) {
			size_t num = archetype->EntityNum();
			size_t chunkNum = archetype->ChunkNum();
			size_t chunkCapacity = archetype->ChunkCapacity();

			for (size_t i = 0; i < chunkNum; i++) {
				size_t idxOffsetInChunk = i * chunkCapacity;
				size_t indexOffsetInQueryChunk = indexOffsetInQuery + idxOffsetInChunk;
				job->emplace([=, singletons = singletons]() {
					(*sys)(
						w,
						SingletonsView{ Span{singletons.data(), singletons.size()} },
						indexOffsetInQueryChunk,
						ChunkView{ archetype, i }
					);
				});
			}

			indexOffsetInQuery += num;
		}
	}
	else {
		auto work = [this, w, sys, singletons = std::move(singletons)]() {
			SingletonsView singletonsView{ Span{singletons.data(), singletons.size()} };

			size_t indexOffsetInQuery = 0;
			for (Archetype* archetype : QueryArchetypes(sys->entityQuery)) {
				size_t num = archetype->EntityNum();
				size_t chunkNum = archetype->ChunkNum();
				size_t chunkCapacity = archetype->ChunkCapacity();

				for (size_t i = 0; i < chunkNum; i++) {
					size_t idxOffsetInChunk = i * chunkCapacity;
					size_t indexOffsetInQueryChunk = indexOffsetInQuery + idxOffsetInChunk;
					(*sys)(
						w,
						singletonsView,
						indexOffsetInQueryChunk,
						ChunkView{ archetype, i }
					);
				}
			}
		};

		if (job)
			job->emplace(std::move(work));
		else
			work();
	}
}

void EntityMngr::GenJob(World* w, Job* job, SystemFunc* sys) const {
	assert(sys->GetMode() == SystemFunc::Mode::Job);

	auto [success, singletons] = LocateSingletons(sys->singletonLocator);
	if (!success)
		return;

	auto work = [=, singletons = std::move(singletons)]() {
		(*sys)(
			w,
			SingletonsView{ Span{singletons.data(), singletons.size()} }
		);
	};

	if (job)
		job->emplace(std::move(work));
	else
		work();
}

void EntityMngr::AutoGen(World* w, Job* job, SystemFunc* sys) const {
	switch (sys->GetMode())
	{
	case SystemFunc::Mode::Entity:
		GenEntityJob(w, job, sys);
		break;
	case SystemFunc::Mode::Chunk:
		GenChunkJob(w, job, sys);
		break;
	case SystemFunc::Mode::Job:
		GenJob(w, job, sys);
		break;
	default:
		assert("not support" && false);
		break;
	}
}

void EntityMngr::Accept(IListener* listener) const {
	// TODO : speed up
	listener->EnterEntityMngr(this);
	for (const auto& [ts, a] : ts2a) {
		for (size_t i = 0; i < a->EntityNum(); i++) {
			auto e = *a->At<Entity>(i);
			listener->EnterEntity(e);
			for (const auto& cmpt : a->Components(i)) {
				listener->EnterCmpt(cmpt);
				listener->ExistCmpt(cmpt);
			}
			listener->ExistEntity(e);
		}
	}
	listener->ExistEntityMngr(this);
}

bool EntityMngr::IsSingleton(CmptType t) const {
	ArchetypeFilter filter{ {CmptAccessType{t}}, {}, {} };
	EntityQuery query{ move(filter) };
	const auto& archetypes = QueryArchetypes(query);
	if (archetypes.size() != 1)
		return false;
	auto* archetype = *archetypes.begin();
	if (archetype->EntityNum() != 1)
		return false;

	return true;
}

Entity EntityMngr::GetSingletonEntity(CmptType t) const {
	assert(IsSingleton(t));
	ArchetypeFilter filter{ {CmptAccessType{t}}, {}, {} };
	EntityQuery query{ move(filter) };
	const auto& archetypes = QueryArchetypes(query);
	auto* archetype = *archetypes.begin();
	return *archetype->At<Entity>(0);
}

CmptPtr EntityMngr::GetSingleton(CmptType t) const {
	ArchetypeFilter filter{ {CmptAccessType{t}}, {}, {} };
	EntityQuery query{ move(filter) };
	const auto& archetypes = QueryArchetypes(query);

	size_t num = 0;
	for (auto* archetype : archetypes)
		num += archetype->EntityNum();

	if (num != 1)
		return { CmptType::Invalid(), nullptr };

	for (auto* archetype : archetypes) {
		if (archetype->EntityNum() != 0)
			return { t, archetype->At(t, 0) };
	}

	return { CmptType::Invalid(), nullptr };
}

std::vector<CmptPtr> EntityMngr::GetCmptArray(const ArchetypeFilter& filter, CmptType type) const {
	assert(filter.all.find(CmptAccessType{ type }) != filter.all.end());

	std::vector<CmptPtr> rst;

	const auto& archetypes = QueryArchetypes(EntityQuery{ filter });

	size_t num = 0;
	for (const auto& archetype : archetypes)
		num += archetype->EntityNum();

	rst.reserve(num);
	for (auto* archetype : archetypes) {
		/*for (size_t i = 0; i < archetype->EntityNum(); i++)
			rst[idx++] = *archetype->At(type, i);*/

		// speed up

		size_t size = archetype->cmptTraits.Sizeof(type);
		size_t offset = archetype->Offsetof(type);
		for (size_t c = 0; c < archetype->chunks.size(); c++) {
			auto* buffer = archetype->chunks[c]->Data();
			auto* beg = buffer + offset;
			size_t chunkSize = archetype->EntityNumOfChunk(c);
			for (size_t i = 0; i < chunkSize; i++)
				rst.emplace_back(type, beg + i * size);
		}
	}

	return rst;
}

std::vector<Entity> EntityMngr::GetEntityArray(const ArchetypeFilter& filter) const {
	std::vector<Entity> rst;

	const auto& archetypes = QueryArchetypes(EntityQuery{ filter });

	size_t num = 0;
	for (const auto& archetype : archetypes)
		num += archetype->EntityNum();

	rst.reserve(num);
	for (auto* archetype : archetypes) {
		/*for (size_t i = 0; i < archetype->EntityNum(); i++)
			rst[idx++] = *archetype->At<Entity>(i);*/

		// speed up

		size_t offset = archetype->Offsetof(CmptType::Of<Entity>);
		for (size_t c = 0; c < archetype->chunks.size(); c++) {
			auto* buffer = archetype->chunks[c]->Data();
			auto* beg = buffer + offset;
			size_t chunkSize = archetype->EntityNumOfChunk(c);
			for (size_t i = 0; i < chunkSize; i++)
				rst.push_back(*reinterpret_cast<Entity*>(beg + i * sizeof(Entity)));
		}
	}

	return rst;
}
