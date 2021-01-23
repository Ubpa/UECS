#include <UECS/EntityMngr.h>

#include <UECS/details/SystemFunc.h>
#include <UECS/IListener.h>

using namespace Ubpa::UECS;
using namespace std;

EntityMngr::EntityMngr()
	: rsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>() }
{}

EntityMngr::EntityMngr(const EntityMngr& em)
	:
	cmptTraits{ em.cmptTraits },
	rsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>() }
{
	ts2a.reserve(em.ts2a.size());
	for (const auto& [ts, a] : em.ts2a) {
		auto [iter, success] = ts2a.try_emplace(ts, std::make_unique<Archetype>(std::pmr::polymorphic_allocator<Chunk>{rsrc.get()}, * a));
		assert(success);
	}
	entityTableFreeEntry = em.entityTableFreeEntry;
	entityTable.resize(em.entityTable.size());
	for (std::size_t i = 0; i < em.entityTable.size(); i++) {
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

	auto& info = entityTable[e.Idx()];
	info.archetype = nullptr;
	info.idxInArchetype = static_cast<std::size_t>(-1);
	info.version++;

	entityTableFreeEntry.push_back(e.Idx());
}

Archetype* EntityMngr::GetOrCreateArchetypeOf(std::span<const TypeID> types) {
	auto typeset = Archetype::GenTypeIDSet(types);
	auto target = ts2a.find(typeset);
	if (target != ts2a.end())
		return target->second.get();

	auto* archetype = Archetype::New(cmptTraits, std::pmr::polymorphic_allocator<Chunk>{rsrc.get()}, types);

	ts2a.emplace(std::move(typeset), std::unique_ptr<Archetype>{ archetype });
	for (auto& [query, archetypes] : queryCache) {
		if (archetype->GetTypeIDSet().IsMatch(query))
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
	info.idxInArchetype = archetype->Create(cmptTraits, e);
	return e;
}

Archetype* EntityMngr::AttachWithoutInit(Entity e, std::span<const TypeID> types) {
	assert(IsSet(types));
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	auto& info = entityTable[e.Idx()];
	Archetype* srcArchetype = info.archetype;
	std::size_t srcIdxInArchetype = info.idxInArchetype;

	const auto& srcTypeIDSet = srcArchetype->GetTypeIDSet();
	auto dstTypeIDSet = srcTypeIDSet;
	dstTypeIDSet.Insert(types);

	// get dstArchetype
	Archetype* dstArchetype;
	auto target = ts2a.find(dstTypeIDSet);
	if (target == ts2a.end()) {
		dstArchetype = Archetype::Add(cmptTraits, srcArchetype, types);
		assert(dstTypeIDSet == dstArchetype->GetTypeIDSet());
		for (auto& [query, archetypes] : queryCache) {
			if (dstTypeIDSet.IsMatch(query))
				archetypes.insert(dstArchetype);
		}
		ts2a.emplace(std::move(dstTypeIDSet), std::unique_ptr<Archetype>{dstArchetype});
	}
	else {
		dstArchetype = target->second.get();
		if (dstArchetype == srcArchetype)
			return srcArchetype;
	}

	// move src to dst
	std::size_t dstIdxInArchetype = dstArchetype->RequestBuffer();

	const auto& srcCmptTraits = srcArchetype->GetArchetypeCmptTraits();
	for (const auto& type : srcTypeIDSet.data) {
		auto* srcCmpt = srcArchetype->At(type, srcIdxInArchetype);
		auto* dstCmpt = dstArchetype->At(type, dstIdxInArchetype);
		srcCmptTraits.MoveConstruct(type, dstCmpt, srcCmpt);
	}

	// erase
	auto srcMovedEntityIndex = srcArchetype->Erase(srcIdxInArchetype);
	if (srcMovedEntityIndex != static_cast<std::size_t>(-1))
		entityTable[srcMovedEntityIndex].idxInArchetype = srcIdxInArchetype;

	info.archetype = dstArchetype;
	info.idxInArchetype = dstIdxInArchetype;

	return srcArchetype;
}

void EntityMngr::Attach(Entity e, std::span<const TypeID> types) {
	auto* origArchetype = AttachWithoutInit(e, types);
	const auto& info = entityTable[e.Idx()];
	for (const auto& type : types) {
		if (origArchetype->GetTypeIDSet().Contains(type))
			continue;

		auto target = cmptTraits.GetDefaultConstructors().find(type);
		if (target == cmptTraits.GetDefaultConstructors().end())
			continue;

		target->second(info.archetype->At(type, info.idxInArchetype));
	}
}

void EntityMngr::Detach(Entity e, std::span<const TypeID> types) {
	assert(IsSet(types));
	if (!Exist(e)) throw std::invalid_argument("EntityMngr::Detach: Entity is invalid");

	auto& info = entityTable[e.Idx()];
	Archetype* srcArchetype = info.archetype;

	const auto& srcTypeIDSet = srcArchetype->GetTypeIDSet();
	auto dstTypeIDSet = srcTypeIDSet;
	dstTypeIDSet.Erase(types);

	// get dstArchetype
	Archetype* dstArchetype;
	bool isNewArchetype;
	auto target = ts2a.find(dstTypeIDSet);
	if (target == ts2a.end()) {
		isNewArchetype = true;
		dstArchetype = Archetype::Remove(srcArchetype, types);
		assert(dstTypeIDSet == dstArchetype->GetTypeIDSet());
		for (auto& [query, archetypes] : queryCache) {
			if (dstTypeIDSet.IsMatch(query))
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
	std::size_t srcIdxInArchetype = info.idxInArchetype;
	std::size_t dstIdxInArchetype = dstArchetype->RequestBuffer();

	const auto& srcCmptTraits = srcArchetype->GetArchetypeCmptTraits();
	for (const auto& type : dstTypeIDSet.data) {
		auto* srcCmpt = srcArchetype->At(type, srcIdxInArchetype);
		auto* dstCmpt = dstArchetype->At(type, dstIdxInArchetype);
		srcCmptTraits.MoveConstruct(type, dstCmpt, srcCmpt);
	}

	// erase
	std::size_t srcMovedEntityIndex = srcArchetype->Erase(srcIdxInArchetype); // call destructor
	if (srcMovedEntityIndex != static_cast<std::size_t>(-1))
		entityTable[srcMovedEntityIndex].idxInArchetype = srcIdxInArchetype;

	info.archetype = dstArchetype;
	info.idxInArchetype = dstIdxInArchetype;

	if(isNewArchetype)
		ts2a.emplace(std::move(dstTypeIDSet), std::unique_ptr<Archetype>{dstArchetype});
}

vector<CmptPtr> EntityMngr::Components(Entity e) const {
	if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

	const auto& info = entityTable[e.Idx()];
	return info.archetype->Components(info.idxInArchetype);
}

Entity EntityMngr::Instantiate(Entity srcEntity) {
	if (!Exist(srcEntity)) throw std::invalid_argument("Entity is invalid");

	std::size_t dstEntityIndex = RequestEntityFreeEntry();
	const auto& srcInfo = entityTable[srcEntity.Idx()];
	auto& dstInfo = entityTable[dstEntityIndex];
	Entity dstEntity{ dstEntityIndex, dstInfo.version };
	std::size_t dstIndexInArchetype = srcInfo.archetype->Instantiate(dstEntity, srcInfo.idxInArchetype);
	dstInfo.archetype = srcInfo.archetype;
	dstInfo.idxInArchetype = dstIndexInArchetype;
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
		if (a->GetTypeIDSet().IsMatch(query))
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

	auto info = entityTable[e.Idx()];
	auto* archetype = info.archetype;
	auto idxInArchetype = info.idxInArchetype;

	auto movedEntityIndex = archetype->Erase(idxInArchetype);

	if (movedEntityIndex != static_cast<std::size_t>(-1))
		entityTable[movedEntityIndex].idxInArchetype = idxInArchetype;

	RecycleEntityEntry(e);
}

vector<CmptAccessPtr> EntityMngr::LocateSingletons(const SingletonLocator& locator) const {
	std::size_t numSingletons = 0;
	vector<CmptAccessPtr> rst;
	rst.reserve(locator.SingletonTypes().size());
	for (const auto& t : locator.SingletonTypes()) {
		auto ptr = GetSingleton(t);
		if (ptr.Ptr() == nullptr)
			return {};
		rst.emplace_back(ptr, t.GetAccessMode());
	}
	return rst;
}

void EntityMngr::GenEntityJob(World* w, Job* job, SystemFunc* sys) const {
	assert(sys->GetMode() == SystemFunc::Mode::Entity);

	auto singletons = LocateSingletons(sys->singletonLocator);
	if (!sys->singletonLocator.SingletonTypes().empty() && singletons.empty())
		return;
	
	if (sys->IsParallel()) {
		assert(job);
		std::size_t indexOffsetInQuery = 0;
		for (Archetype* archetype : QueryArchetypes(sys->entityQuery)) {
			auto [chunkEntity, chunkCmpts, sizes] = archetype->Locate(sys->entityQuery.locator);

			std::size_t num = archetype->EntityNum();
			std::size_t chunkNum = archetype->ChunkNum();
			std::size_t chunkCapacity = archetype->ChunkCapacity();

			for (std::size_t i = 0; i < chunkNum; i++) {
				job->emplace([=, sizes = sizes, entities = chunkEntity[i], cmpts = move(chunkCmpts[i]), singletons = singletons]() mutable {
					std::size_t idxOffsetInChunk = i * chunkCapacity;
					std::size_t indexOffsetInQueryChunk = indexOffsetInQuery + idxOffsetInChunk;
					CmptsView chunkView{ std::span{cmpts.data(), cmpts.size()} };
					SingletonsView singletonsView{ std::span{singletons.data(), singletons.size()} };

					std::size_t J = min(chunkCapacity, num - idxOffsetInChunk);
					for (std::size_t j = 0; j < J; j++) {
						(*sys)(
							w,
							singletonsView,
							entities[j],
							indexOffsetInQueryChunk + j,
							chunkView
						);
						for (std::size_t k = 0; k < cmpts.size(); k++)
							reinterpret_cast<uint8_t*&>(cmpts[k].p) += sizes[k];
					}
				});
			}

			indexOffsetInQuery += num;
		}
	}
	else {
		auto work = [this, singletons = std::move(singletons), sys, w]() {
			std::size_t indexOffsetInQuery = 0;
			for (Archetype* archetype : QueryArchetypes(sys->entityQuery)) {
				auto [chunkEntity, chunkCmpts, sizes] = archetype->Locate(sys->entityQuery.locator);

				std::size_t num = archetype->EntityNum();
				std::size_t chunkNum = archetype->ChunkNum();
				std::size_t chunkCapacity = archetype->ChunkCapacity();

				for (std::size_t i = 0; i < chunkNum; i++) {
					std::size_t idxOffsetInChunk = i * chunkCapacity;
					std::size_t indexOffsetInQueryChunk = indexOffsetInQuery + idxOffsetInChunk;
					CmptsView chunkView{ std::span{chunkCmpts[i].data(), chunkCmpts[i].size()} };
					SingletonsView singletonsView{ std::span{singletons.data(), singletons.size()} };

					std::size_t J = min(chunkCapacity, num - idxOffsetInChunk);
					for (std::size_t j = 0; j < J; j++) {
						(*sys)(
							w,
							singletonsView,
							chunkEntity[i][j],
							indexOffsetInQueryChunk + j,
							chunkView
							);
						for (std::size_t k = 0; k < chunkCmpts[i].size(); k++)
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

	auto singletons = LocateSingletons(sys->singletonLocator);
	if (!sys->singletonLocator.SingletonTypes().empty() && singletons.empty())
		return;

	if (sys->IsParallel()) {
		assert(job != nullptr);
		std::size_t indexOffsetInQuery = 0;
		for (Archetype* archetype : QueryArchetypes(sys->entityQuery)) {
			std::size_t num = archetype->EntityNum();
			std::size_t chunkNum = archetype->ChunkNum();
			std::size_t chunkCapacity = archetype->ChunkCapacity();

			for (std::size_t i = 0; i < chunkNum; i++) {
				std::size_t idxOffsetInChunk = i * chunkCapacity;
				std::size_t indexOffsetInQueryChunk = indexOffsetInQuery + idxOffsetInChunk;
				job->emplace([=, singletons = singletons]() {
					(*sys)(
						w,
						SingletonsView{ std::span{singletons.data(), singletons.size()} },
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
			SingletonsView singletonsView{ std::span{singletons.data(), singletons.size()} };

			std::size_t indexOffsetInQuery = 0;
			for (Archetype* archetype : QueryArchetypes(sys->entityQuery)) {
				std::size_t num = archetype->EntityNum();
				std::size_t chunkNum = archetype->ChunkNum();
				std::size_t chunkCapacity = archetype->ChunkCapacity();

				for (std::size_t i = 0; i < chunkNum; i++) {
					std::size_t idxOffsetInChunk = i * chunkCapacity;
					std::size_t indexOffsetInQueryChunk = indexOffsetInQuery + idxOffsetInChunk;
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

	auto singletons = LocateSingletons(sys->singletonLocator);
	if (!sys->singletonLocator.SingletonTypes().empty() && singletons.empty())
		return;

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
		for (std::size_t i = 0; i < a->EntityNum(); i++) {
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
	return *archetype->At<Entity>(0);
}

CmptPtr EntityMngr::GetSingleton(TypeID t) const {
	ArchetypeFilter filter{ {AccessTypeID{t}}, {}, {} };
	EntityQuery query{ move(filter) };
	const auto& archetypes = QueryArchetypes(query);

	std::size_t num = 0;
	for (auto* archetype : archetypes)
		num += archetype->EntityNum();

	if (num != 1)
		return { TypeID{}, nullptr };

	for (auto* archetype : archetypes) {
		if (archetype->EntityNum() != 0)
			return { t, archetype->At(t, 0) };
	}

	return { TypeID{}, nullptr };
}

std::vector<CmptPtr> EntityMngr::GetCmptArray(const ArchetypeFilter& filter, TypeID type) const {
	assert(filter.all.find(AccessTypeID{ type }) != filter.all.end());

	std::vector<CmptPtr> rst;

	const auto& archetypes = QueryArchetypes(EntityQuery{ filter });

	std::size_t num = 0;
	for (const auto& archetype : archetypes)
		num += archetype->EntityNum();

	rst.reserve(num);
	for (auto* archetype : archetypes) {
		/*for (std::size_t i = 0; i < archetype->EntityNum(); i++)
			rst[idx++] = *archetype->At(type, i);*/

		// speed up

		std::size_t size = archetype->cmptTraits.Sizeof(type);
		std::size_t offset = archetype->Offsetof(type);
		for (std::size_t c = 0; c < archetype->chunks.size(); c++) {
			auto* buffer = archetype->chunks[c]->Data();
			auto* beg = buffer + offset;
			std::size_t chunkSize = archetype->EntityNumOfChunk(c);
			for (std::size_t i = 0; i < chunkSize; i++)
				rst.emplace_back(type, beg + i * size);
		}
	}

	return rst;
}

std::vector<Entity> EntityMngr::GetEntityArray(const ArchetypeFilter& filter) const {
	std::vector<Entity> rst;

	const auto& archetypes = QueryArchetypes(EntityQuery{ filter });

	std::size_t num = 0;
	for (const auto& archetype : archetypes)
		num += archetype->EntityNum();

	rst.reserve(num);
	for (auto* archetype : archetypes) {
		/*for (std::size_t i = 0; i < archetype->EntityNum(); i++)
			rst[idx++] = *archetype->At<Entity>(i);*/

		// speed up

		std::size_t offset = archetype->Offsetof(TypeID_of<Entity>);
		for (std::size_t c = 0; c < archetype->chunks.size(); c++) {
			auto* buffer = archetype->chunks[c]->Data();
			auto* beg = buffer + offset;
			std::size_t chunkSize = archetype->EntityNumOfChunk(c);
			for (std::size_t i = 0; i < chunkSize; i++)
				rst.push_back(*reinterpret_cast<Entity*>(beg + i * sizeof(Entity)));
		}
	}

	return rst;
}
