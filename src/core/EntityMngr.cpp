#include <UECS/EntityMngr.hpp>

#include "Archetype.hpp"

#include <UECS/SystemFunc.hpp>
#include <UECS/IListener.hpp>
#include <UECS/CommandBuffer.hpp>

using namespace Ubpa::UECS;
using namespace std;

EntityMngr::EntityMngr(World* world) :
	world{ world },
	cmptTraits{ world->GetUnsyncResource() } {}

EntityMngr::EntityMngr(const EntityMngr& em, World* world) :
	cmptTraits{ em.cmptTraits, world->GetUnsyncResource() },
	world{ world }
{
	ts2a.reserve(em.ts2a.size());
	for (const auto& [ts, a] : em.ts2a)
		ts2a.try_emplace(ts, std::make_unique<Archetype>(*a, world));
	entityTableFreeEntry = em.entityTableFreeEntry;
	entityTable.resize(em.entityTable.size());
	for (std::size_t i = 0; i < em.entityTable.size(); i++) {
		auto& dst = entityTable[i];
		const auto& src = em.entityTable[i];
		dst.chunkIdx = src.chunkIdx;
		dst.idxInChunk = src.idxInChunk;
		dst.version = src.version;
		if (src.archetype)
			dst.archetype = ts2a.at(src.archetype->cmptTraits.GetTypes()).get();
	}
	queryCache.reserve(em.queryCache.size());
	for (const auto& [query, srcArchetypes] : em.queryCache) {
		auto& dstArchetypes = queryCache[query];
		for (auto* archetype : srcArchetypes)
			dstArchetypes.insert(ts2a.at(archetype->GetCmptTraits().GetTypes()).get());
	}
}

EntityMngr::EntityMngr(EntityMngr&& em, World* world) noexcept :
	cmptTraits{ std::move(em.cmptTraits) },
	queryCache{std::move(em.queryCache)},
	entityTable{std::move(em.entityTable)},
	ts2a{std::move(em.ts2a)},
	entityTableFreeEntry{std::move(em.entityTableFreeEntry)},
	world{ world } {}

EntityMngr::~EntityMngr() {
	ts2a.clear();
}

std::size_t EntityMngr::TypeIDSetHash::operator()(const small_flat_set<TypeID>& types) const noexcept {
	std::size_t rst = TypeID_of<small_flat_set<TypeID>>.GetValue();
	for (const auto& t : types)
		rst = hash_combine(rst, t.GetValue());
	return rst;
}

void EntityMngr::Clear() {
	entityTableFreeEntry.clear();
	entityTable.clear();
	queryCache.clear();
	ts2a.clear();
}

bool EntityMngr::Have(Entity e, TypeID type) const {
	assert(!type.Is<Entity>());
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");
	return entityTable[e.index].archetype->GetCmptTraits().GetTypes().contains(type);
}

CmptAccessPtr EntityMngr::AccessComponent(Entity e, AccessTypeID type) const {
	assert(!type.Is<Entity>());
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");
	const auto& info = entityTable[e.index];
	return info.archetype->At(type, { info.chunkIdx, info.idxInChunk });
}

CmptAccessPtr EntityMngr::WriteComponent(Entity e, TypeID t) const { return AccessComponent(e, { t, AccessMode::WRITE }); }
CmptAccessPtr EntityMngr::ReadComponent(Entity e, TypeID t) const { return AccessComponent(e, { t, AccessMode::LATEST }); }

bool EntityMngr::Exist(Entity e) const noexcept {
	return e.index < entityTable.size() && e.version == entityTable[e.index].version && entityTable[e.index].archetype;
}

std::size_t EntityMngr::RequestEntityFreeEntry() {
	if (entityTableFreeEntry.empty()) {
		std::size_t index = entityTable.size();
		entityTable.emplace_back();
		return index;
	}

	std::size_t entry = entityTableFreeEntry.back();
	entityTableFreeEntry.pop_back();
	return entry;
}

void EntityMngr::RecycleEntityEntry(Entity e) {
	assert(Exist(e));

	auto& info = entityTable[e.index];
	info.archetype = nullptr;
	info.chunkIdx = static_cast<std::size_t>(-1);
	info.idxInChunk = static_cast<std::size_t>(-1);
	info.version++;

	entityTableFreeEntry.push_back(e.index);
}

Archetype* EntityMngr::GetOrCreateArchetypeOf(std::span<const TypeID> types) {
	auto typeset = Archetype::GenTypeIDSet(types);
	auto target = ts2a.find(typeset);
	if (target != ts2a.end())
		return target->second.get();

	auto* archetype = Archetype::New(cmptTraits, world, types);

	ts2a.emplace(std::move(typeset), std::unique_ptr<Archetype>{ archetype });
	for (auto& [query, archetypes] : queryCache) {
		if (query.IsMatch(archetype->GetCmptTraits().GetTypes()))
			archetypes.insert(archetype);
	}

	return archetype;
}

Entity EntityMngr::Create(std::span<const TypeID> types) {
	Archetype* archetype = GetOrCreateArchetypeOf(types);
	std::size_t entityIndex = RequestEntityFreeEntry();
	auto& info = entityTable[entityIndex];
	Entity e{ entityIndex, info.version };
	info.archetype = archetype;
	auto addr = archetype->Create(e);
	info.chunkIdx = addr.chunkIdx;
	info.idxInChunk = addr.idxInChunk;
	return e;
}

void EntityMngr::Attach(Entity e, std::span<const TypeID> types) {
	assert(IsSet(types));
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	auto& info = entityTable[e.index];
	Archetype* srcArchetype = info.archetype;
	std::size_t srcChunkIdx = info.chunkIdx;
	std::size_t srcIdxInChunk = info.idxInChunk;

	const auto& srcTypeIDSet = srcArchetype->GetCmptTraits().GetTypes();
	auto dstTypeIDSet = srcTypeIDSet;
	dstTypeIDSet.insert(types.begin(), types.end());

	// get dstArchetype
	Archetype* dstArchetype;
	auto target = ts2a.find(dstTypeIDSet);
	if (target == ts2a.end()) {
		dstArchetype = Archetype::Add(cmptTraits, srcArchetype, types);
		assert(dstTypeIDSet == dstArchetype->GetCmptTraits().GetTypes());
		for (auto& [query, archetypes] : queryCache) {
			if (query.IsMatch(dstTypeIDSet))
				archetypes.insert(dstArchetype);
		}
		ts2a.emplace(std::move(dstTypeIDSet), std::unique_ptr<Archetype>{dstArchetype});
	}
	else {
		dstArchetype = target->second.get();
		if (dstArchetype == srcArchetype)
			return;
	}

	// move src to dst
	auto dstAddr = dstArchetype->RequestBuffer();
	std::size_t dstChunkIdx = dstAddr.chunkIdx;
	std::size_t dstidxInChunk = dstAddr.idxInChunk;

	const auto& srcCmptTraits = srcArchetype->GetCmptTraits();
	for (const auto& type : srcTypeIDSet) {
		auto* srcCmpt = srcArchetype->ReadAt(type, { srcChunkIdx, srcIdxInChunk }).Ptr();
		auto* dstCmpt = dstArchetype->WriteAt(type, { dstChunkIdx, dstidxInChunk }).Ptr();
		srcCmptTraits.GetTrait(type).MoveConstruct(dstCmpt, srcCmpt, dstArchetype->GetChunks()[dstChunkIdx]->GetChunkUnsyncResource());
	}

	// erase
	auto srcMovedEntityIndex = srcArchetype->Erase({ srcChunkIdx, srcIdxInChunk });
	if (srcMovedEntityIndex != static_cast<std::size_t>(-1)) {
		entityTable[srcMovedEntityIndex].chunkIdx = srcChunkIdx;
		entityTable[srcMovedEntityIndex].idxInChunk = srcIdxInChunk;
	}

	for (const auto& type : types) {
		auto target = dstArchetype->GetCmptTraits().GetTypes().find(type);
		assert(target != dstArchetype->GetCmptTraits().GetTypes().end());
		auto idx = static_cast<std::size_t>(std::distance(dstArchetype->GetCmptTraits().GetTypes().begin(), target));
		dstArchetype->GetCmptTraits().GetTraits()[idx].DefaultConstruct(
			dstArchetype->WriteAt(type, { dstChunkIdx, dstidxInChunk }).Ptr(),
			dstArchetype->chunks[dstChunkIdx]->GetChunkUnsyncResource());
	}

	info.archetype = dstArchetype;
	info.chunkIdx = dstChunkIdx;
	info.idxInChunk = dstidxInChunk;
}

void EntityMngr::Detach(Entity e, std::span<const TypeID> types) {
	assert(IsSet(types));
	if (!Exist(e)) throw std::invalid_argument("EntityMngr::Detach: Entity is invalid");

	auto& info = entityTable[e.index];
	Archetype* srcArchetype = info.archetype;

	const auto& srcTypeIDSet = srcArchetype->GetCmptTraits().GetTypes();
	auto dstTypeIDSet = srcTypeIDSet;
	for (const auto& t : types)
		dstTypeIDSet.erase(t);

	// get dstArchetype
	Archetype* dstArchetype;
	bool isNewArchetype;
	auto target = ts2a.find(dstTypeIDSet);
	if (target == ts2a.end()) {
		isNewArchetype = true;
		dstArchetype = Archetype::Remove(srcArchetype, types);
		assert(dstTypeIDSet == dstArchetype->GetCmptTraits().GetTypes());
		for (auto& [query, archetypes] : queryCache) {
			if (query.IsMatch(dstTypeIDSet))
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
	std::size_t srcChunkIdx = info.chunkIdx;
	std::size_t srcIdxInChunk = info.idxInChunk;
	auto dstAddr = dstArchetype->RequestBuffer();
	std::size_t dstChunkIdx = dstAddr.chunkIdx;
	std::size_t dstidxInChunk = dstAddr.idxInChunk;

	const auto& srcCmptTraits = srcArchetype->GetCmptTraits();
	for (const auto& type : dstTypeIDSet) {
		auto* srcCmpt = srcArchetype->ReadAt(type, { srcChunkIdx, srcIdxInChunk }).Ptr();
		auto* dstCmpt = dstArchetype->WriteAt(type, { dstChunkIdx, dstidxInChunk }).Ptr();
		srcCmptTraits.GetTrait(type).MoveConstruct(dstCmpt, srcCmpt, dstArchetype->GetChunks()[dstChunkIdx]->GetChunkUnsyncResource());
	}

	// erase
	std::size_t srcMovedEntityIndex = srcArchetype->Erase({ srcChunkIdx, srcIdxInChunk }); // call destructor
	if (srcMovedEntityIndex != static_cast<std::size_t>(-1)) {
		entityTable[srcMovedEntityIndex].chunkIdx = srcChunkIdx;
		entityTable[srcMovedEntityIndex].idxInChunk = srcIdxInChunk;
	}

	info.archetype = dstArchetype;
	info.chunkIdx = dstChunkIdx;
	info.idxInChunk = dstidxInChunk;

	if(isNewArchetype)
		ts2a.emplace(std::move(dstTypeIDSet), std::unique_ptr<Archetype>{dstArchetype});
}

vector<CmptAccessPtr> EntityMngr::AccessComponents(Entity e, AccessMode mode) const {
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	const auto& info = entityTable[e.index];
	return info.archetype->AccessComponents({info.chunkIdx, info.idxInChunk}, mode);
}

std::vector<CmptAccessPtr> EntityMngr::WriteComponents(Entity e) const { return AccessComponents(e, AccessMode::WRITE); }
std::vector<CmptAccessPtr> EntityMngr::ReadComponents(Entity e) const { return AccessComponents(e, AccessMode::LATEST); }

Entity EntityMngr::Instantiate(Entity srcEntity) {
	if (!Exist(srcEntity)) throw std::invalid_argument("Entity is invalid");

	std::size_t dstEntityIndex = RequestEntityFreeEntry();
	const auto& srcInfo = entityTable[srcEntity.index];
	auto& dstInfo = entityTable[dstEntityIndex];
	Entity dstEntity{ dstEntityIndex, dstInfo.version };
	auto dstAddr = srcInfo.archetype->Instantiate(dstEntity, { srcInfo.chunkIdx, srcInfo.idxInChunk });
	std::size_t dstChunkIdx = dstAddr.chunkIdx;
	std::size_t dstIdxInChunk = dstAddr.idxInChunk;
	dstInfo.archetype = srcInfo.archetype;
	dstInfo.chunkIdx = dstChunkIdx;
	dstInfo.idxInChunk = dstIdxInChunk;
	return dstEntity;
}

bool EntityMngr::IsSet(std::span<const TypeID> types) noexcept {
	for (std::size_t i = 0; i < types.size(); i++) {
		for (std::size_t j = 0; j < i; j++)
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
		if (query.IsMatch(a->GetCmptTraits().GetTypes()))
			archetypes.insert(a.get());
	}

	return archetypes;
}

std::size_t EntityMngr::EntityNum(const EntityQuery& query) const {
	std::size_t sum = 0;
	for (const auto& archetype : QueryArchetypes(query))
		sum += archetype->EntityNum();
	return sum;
}

void EntityMngr::Destroy(Entity e) {
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	auto info = entityTable[e.index];
	auto* archetype = info.archetype;

	auto movedEntityIndex = archetype->Erase({ info.chunkIdx, info.idxInChunk });

	if (movedEntityIndex != static_cast<std::size_t>(-1)) {
		entityTable[movedEntityIndex].chunkIdx = info.chunkIdx;
		entityTable[movedEntityIndex].idxInChunk = info.idxInChunk;
	}

	RecycleEntityEntry(e);
}

std::size_t EntityMngr::TotalEntityNum() const noexcept
{ return entityTable.size() - entityTableFreeEntry.size(); }

std::span<const std::size_t> EntityMngr::GetEntityFreeEntries() const noexcept
{ return { entityTableFreeEntry.data(), entityTableFreeEntry.size() }; }

std::size_t EntityMngr::GetEntityVersion(std::size_t idx) const noexcept { return entityTable[idx].version; }

Ubpa::small_vector<CmptAccessPtr> EntityMngr::LocateSingletons(const SingletonLocator& locator) const {
	std::size_t numSingletons = 0;
	small_vector<CmptAccessPtr> rst;
	rst.reserve(locator.SingletonTypes().size());
	for (const auto& t : locator.SingletonTypes()) {
		auto ptr = AccessSingleton(t);
		if (ptr.Ptr() == nullptr)
			return {};
		rst.push_back(ptr);
	}
	return rst;
}

bool EntityMngr::GenEntityJob(World* w, Job* job, SystemFunc* sys, int layer) const {
	assert(sys->GetMode() == SystemFunc::Mode::Entity);

	auto singletons = LocateSingletons(sys->singletonLocator);
	if (!sys->singletonLocator.SingletonTypes().empty() && singletons.empty())
		return false;

	const auto& archetypes = QueryArchetypes(sys->entityQuery);
	if (archetypes.empty())
		return false;

	if (sys->IsParallel()) {
		assert(job);
		std::size_t indexOffsetInQuery = 0;
		for (Archetype* archetype : archetypes) {
			std::size_t indexOffsetInArchetype = 0;
			auto chunks = archetype->GetChunks();
			for (std::size_t i = 0; i < chunks.size(); i++) {
				chunks[i]->ApplyChanges({ sys->entityQuery.locator.AccessTypeIDs().data(), sys->entityQuery.locator.AccessTypeIDs().size() });
				chunks[i]->ApplyChanges({ sys->randomAccessor.types.data(), sys->randomAccessor.types.size() });
				if (!sys->changeFilter.types.empty() && !chunks[i]->HasAnyChange(sys->changeFilter.types, world->Version()))
					continue;
				if (chunks[i]->EntityNum() == 0)
					continue;
				job->emplace([=, chunk = chunks[i], singletons = singletons]() {
					auto [entities, cmpts, sizes] = chunk->Locate({ sys->entityQuery.locator.AccessTypeIDs().data(), sys->entityQuery.locator.AccessTypeIDs().size() });
					std::size_t indexOffsetInChunk = indexOffsetInQuery + indexOffsetInArchetype;
					CmptsView cmptsView{ std::span{cmpts.data(), cmpts.size()} };
					SingletonsView singletonsView{ std::span{singletons.data(), singletons.size()} };
					CommandBuffer cb;
					for (std::size_t j = 0; j < chunk->EntityNum(); j++) {
						(*sys)(
							w,
							singletonsView,
							entities[j],
							indexOffsetInChunk + j,
							cmptsView,
							&cb
						);
						for (std::size_t k = 0; k < cmpts.size(); k++)
							reinterpret_cast<uint8_t*&>(cmpts[k].p) += sizes[k];
					}
					w->AddCommandBuffer(std::move(cb), layer);
				});
				indexOffsetInArchetype += chunks[i]->EntityNum();
			}
			indexOffsetInQuery += indexOffsetInArchetype;
		}
	}
	else {
		auto work = [this, singletons = std::move(singletons), sys, w, archetypes, layer]() {
			CommandBuffer cb;
			std::size_t indexOffsetInQuery = 0;
			for (Archetype* archetype : archetypes) {
				std::size_t indexOffsetInArchetype = 0;
				auto chunks = archetype->GetChunks();

				for (std::size_t i = 0; i < chunks.size(); i++) {
					chunks[i]->ApplyChanges({ sys->entityQuery.locator.AccessTypeIDs().data(), sys->entityQuery.locator.AccessTypeIDs().size() });
					chunks[i]->ApplyChanges({ sys->randomAccessor.types.data(), sys->randomAccessor.types.size() });
					if (!sys->changeFilter.types.empty() && !archetype->chunks[i]->HasAnyChange(sys->changeFilter.types, world->Version()))
						continue;
					if (chunks[i]->EntityNum() == 0)
						continue;

					auto [entities, cmpts, sizes] = chunks[i]->Locate({ sys->entityQuery.locator.AccessTypeIDs().data(), sys->entityQuery.locator.AccessTypeIDs().size() });
					std::size_t indexOffsetInChunk = indexOffsetInQuery + indexOffsetInArchetype;
					CmptsView cmptsView{ std::span{cmpts.data(), cmpts.size()} };
					SingletonsView singletonsView{ std::span{singletons.data(), singletons.size()} };

					for (std::size_t j = 0; j < chunks[i]->EntityNum(); j++) {
						(*sys)(
							w,
							singletonsView,
							entities[j],
							indexOffsetInChunk + j,
							cmptsView,
							&cb
							);
						for (std::size_t k = 0; k < sys->entityQuery.locator.AccessTypeIDs().size(); k++)
							reinterpret_cast<uint8_t*&>(cmpts[k].p) += sizes[k];
					}
					indexOffsetInArchetype += chunks[i]->EntityNum();
				}
				indexOffsetInQuery += indexOffsetInArchetype;
			}
			w->AddCommandBuffer(std::move(cb), layer);
		};

		if (job)
			job->emplace(std::move(work));
		else
			work();
	}

	return true;
}

bool EntityMngr::GenChunkJob(World* w, Job* job, SystemFunc* sys, int layer) const {
	assert(sys->GetMode() == SystemFunc::Mode::Chunk);

	auto singletons = LocateSingletons(sys->singletonLocator);
	if (!sys->singletonLocator.SingletonTypes().empty() && singletons.empty())
		return false;

	if (sys->IsParallel()) {
		assert(job != nullptr);
		const auto& archetypes = QueryArchetypes(sys->entityQuery);
		if (archetypes.empty())
			return false;
		std::size_t indexOffsetInQuery = 0;
		for (Archetype* archetype : archetypes) {
			std::size_t indexOffsetInArchetype = 0;
			auto chunks = archetype->GetChunks();

			for (std::size_t i = 0; i < chunks.size(); i++) {
				chunks[i]->ApplyChanges({ sys->entityQuery.filter.all.data(), sys->entityQuery.filter.all.size() });
				chunks[i]->ApplyChanges({ sys->entityQuery.filter.any.data(), sys->entityQuery.filter.any.size() });
				chunks[i]->ApplyChanges({ sys->randomAccessor.types.data(), sys->randomAccessor.types.size() });
				assert(sys->entityQuery.locator.AccessTypeIDs().empty());
				if (!sys->changeFilter.types.empty() && !chunks[i]->HasAnyChange(sys->changeFilter.types, world->Version()))
					continue;
				if (chunks[i]->EntityNum() == 0)
					continue;

				std::size_t indexOffsetInChunk = indexOffsetInQuery + indexOffsetInArchetype;

				job->emplace([=, singletons = singletons]() {
					CommandBuffer cb;
					(*sys)(
						w,
						SingletonsView{ std::span{singletons.data(), singletons.size()} },
						indexOffsetInChunk,
						archetype->chunks[i],
						&cb
					);
					w->AddCommandBuffer(std::move(cb), layer);
				});
				indexOffsetInArchetype += chunks[i]->EntityNum();
			}

			indexOffsetInQuery += indexOffsetInArchetype;
		}
	}
	else {
		auto work = [this, w, sys, singletons = std::move(singletons), layer]() {
			SingletonsView singletonsView{ std::span{singletons.data(), singletons.size()} };

			CommandBuffer cb;
			std::size_t indexOffsetInQuery = 0;
			for (Archetype* archetype : QueryArchetypes(sys->entityQuery)) {
				std::size_t indexOffsetInArchetype = 0;
				auto chunks = archetype->GetChunks();

				for (std::size_t i = 0; i < chunks.size(); i++) {
					chunks[i]->ApplyChanges({ sys->entityQuery.filter.all.data(), sys->entityQuery.filter.all.size() });
					chunks[i]->ApplyChanges({ sys->entityQuery.filter.any.data(), sys->entityQuery.filter.any.size() });
					assert(sys->entityQuery.locator.AccessTypeIDs().empty());
					if (!sys->changeFilter.types.empty() && !archetype->chunks[i]->HasAnyChange(sys->changeFilter.types, world->Version()))
						continue;
					if (chunks[i]->EntityNum() == 0)
						continue;

					std::size_t indexOffsetInChunk = indexOffsetInQuery + indexOffsetInArchetype;
					(*sys)(
						w,
						singletonsView,
						indexOffsetInChunk,
						archetype->chunks[i],
						&cb
					);
					indexOffsetInArchetype += chunks[i]->EntityNum();
				}
				indexOffsetInQuery += indexOffsetInArchetype;
			}
			w->AddCommandBuffer(std::move(cb), layer);
		};

		if (job)
			job->emplace(std::move(work));
		else
			work();
	}

	return true;
}

bool EntityMngr::GenJob(World* w, Job* job, SystemFunc* sys) const {
	assert(sys->GetMode() == SystemFunc::Mode::Job);

	auto singletons = LocateSingletons(sys->singletonLocator);
	if (!sys->singletonLocator.SingletonTypes().empty() && singletons.empty())
		return false;

	auto work = [=, singletons = std::move(singletons)]() {
		(*sys)(
			w,
			SingletonsView{ std::span{singletons.data(), singletons.size()} }
		);
	};

	if (job)
		job->emplace(std::move(work));
	else
		work();

	return true;
}

bool EntityMngr::AutoGen(World* w, Job* job, SystemFunc* sys, int layer) const {
	switch (sys->GetMode())
	{
	case SystemFunc::Mode::Entity:
		return GenEntityJob(w, job, sys, layer);
	case SystemFunc::Mode::Chunk:
		return GenChunkJob(w, job, sys, layer);
	case SystemFunc::Mode::Job:
		return GenJob(w, job, sys);
	default:
		assert("not support" && false);
		return false;
	}
}

void EntityMngr::Accept(IListener* listener) const {
	// TODO : speed up
	listener->EnterEntityMngr(this);
	for (const auto& [ts, a] : ts2a) {
		for (std::size_t i = 0; i < a->GetChunks().size(); i++) {
			for (std::size_t j = 0; j < a->GetChunks()[i]->EntityNum(); j++) {
				auto e = *a->ReadAt<Entity>({ i,j });
				listener->EnterEntity(e);
				for (const auto& cmpt : a->AccessComponents({ i,j }, AccessMode::WRITE)) {
					listener->EnterCmpt({ cmpt.AccessType(), cmpt.Ptr() });
					listener->ExistCmpt({ cmpt.AccessType(), cmpt.Ptr() });
				}
				listener->ExistEntity(e);
			}
		}
	}
	listener->ExistEntityMngr(this);
}

bool EntityMngr::IsSingleton(TypeID t) const {
	ArchetypeFilter filter{ {AccessTypeID{t}}, {}, {} };
	EntityQuery query{ move(filter) };
	const auto& archetypes = QueryArchetypes(query);
	if (archetypes.size() != 1)
		return false;
	auto* archetype = *archetypes.begin();
	if (archetype->EntityNum() != 1)
		return false;

	return true;
}

Entity EntityMngr::GetSingletonEntity(TypeID t) const {
	assert(IsSingleton(t));
	ArchetypeFilter filter{ {AccessTypeID{t}}, {}, {} };
	EntityQuery query{ move(filter) };
	const auto& archetypes = QueryArchetypes(query);
	auto* archetype = *archetypes.begin();
	return *archetype->ReadAt<Entity>({ 0,0 });
}

CmptAccessPtr EntityMngr::AccessSingleton(AccessTypeID access_type) const {
	ArchetypeFilter filter{ {access_type}, {}, {} };
	EntityQuery query{ move(filter) };
	const auto& archetypes = QueryArchetypes(query);

	std::size_t num = 0;
	for (auto* archetype : archetypes)
		num += archetype->EntityNum();

	if (num != 1)
		return { access_type, nullptr };

	for (auto* archetype : archetypes) {
		if (archetype->EntityNum() != 0)
			return archetype->At(access_type, { 0,0 });
	}

	return { access_type, nullptr };
}

CmptAccessPtr EntityMngr::WriteSingleton(TypeID type) const { return AccessSingleton({ type, AccessMode::WRITE }); }
CmptAccessPtr EntityMngr::ReadSingleton(TypeID type) const { return AccessSingleton({ type, AccessMode::LATEST }); }

void EntityMngr::NewFrame() noexcept {
	for (auto& [ts, a] : ts2a)
		a->NewFrame();
}

std::tuple<Chunk*, std::size_t> EntityMngr::GetChunk(Entity e) const {
	if(!Exist(e)) throw std::invalid_argument("Entity is invalid");
	const auto& info = entityTable[e.index];
	return { info.archetype->GetChunks()[info.chunkIdx],info.idxInChunk };
}
