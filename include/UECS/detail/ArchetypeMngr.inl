#pragma once

#include "../CmptTag.h"

#include <UTemplate/Typelist.h>
#include <UTemplate/Func.h>

namespace Ubpa::detail::ArchetypeMngr_ {
	template<typename ArgList, typename OrderList, typename TaggedCmptList>
	struct GenJob;
}

namespace Ubpa {
	template<typename... Cmpts>
	inline Archetype* ArchetypeMngr::GetOrCreateArchetypeOf() {
		auto id = CmptIDSet(TypeList<Cmpts...>{});
		auto target = id2a.find(id);
		if (target == id2a.end()) {
			auto archetype = new Archetype(this, TypeList<Cmpts...>{});
			id2a[id] = archetype;
			ids.insert(archetype->ID());
			return archetype;
		}
		else
			return target->second;
	}

	Archetype* ArchetypeMngr::GetArchetypeOf(const CmptIDSet& archetypeID) {
		auto target = id2a.find(archetypeID);
		assert(target != id2a.end());
		return target->second;
	}

	template<typename... Cmpts>
	const std::tuple<EntityBase*, Cmpts*...> ArchetypeMngr::CreateEntity() {
		auto entity = entityPool.Request();

		Archetype* archetype = GetOrCreateArchetypeOf<Cmpts...>();
		auto [idx, cmpts] = archetype->CreateEntity<Cmpts...>();

		entity->archetype = archetype;
		entity->idx = idx;
		ai2e[{archetype,idx}] = entity;

		using CmptList = TypeList<Cmpts...>;
		return { entity, std::get<Find_v<CmptList, Cmpts>>(cmpts)... };
	}

	template<typename... Cmpts>
	const std::set<Archetype*>& ArchetypeMngr::GetArchetypeWith() {
		constexpr size_t cmptsHash = TypeID<QuickSort_t<TypeList<Cmpts...>, TypeID_Less>>;
		auto target = cmpts2as.find(cmptsHash);
		if (target != cmpts2as.end())
			return target->second;

		std::set<Archetype*>& rst = cmpts2as[cmptsHash];
		cmpts2ids.emplace(cmptsHash, CmptIDSet{ TypeList<Cmpts...>{} });
		for (auto& [id, a] : id2a) {
			if (id.IsContain<Cmpts...>())
				rst.insert(a);
		}

		return rst;
	}

	template<typename... Cmpts>
	const std::tuple<Cmpts*...> ArchetypeMngr::EntityAttach(EntityBase* e) {
		assert(!e->archetype->id.IsContain<Cmpts...>());

		Archetype* srcArchetype = e->archetype;
		size_t srcIdx = e->idx;

		auto& srcID = srcArchetype->ID();
		auto dstID = srcID;
		dstID.Add<Cmpts...>();

		// get dstArchetype
		Archetype* dstArchetype;
		auto target = id2a.find(dstID);
		if (target == id2a.end()) {
			dstArchetype = Archetype::Add<Cmpts...>(srcArchetype);
			assert(dstID == dstArchetype->ID());
			id2a[dstID] = dstArchetype;
			ids.insert(dstID);
			for (auto& [cmptsHash, archetypes] : cmpts2as) {
				if (dstArchetype->ID().IsContain(cmpts2ids[cmptsHash]))
					archetypes.insert(dstArchetype);
			}
		}
		else
			dstArchetype = target->second;

		// move src to dst
		size_t dstIdx = dstArchetype->RequestBuffer();
		
		(new(dstArchetype->At<Cmpts>(dstIdx))Cmpts, ...);
		for (auto cmptHash : srcID) {
			auto [srcCmpt, srcSize] = srcArchetype->At(cmptHash, srcIdx);
			auto [dstCmpt, dstSize] = dstArchetype->At(cmptHash, dstIdx);
			assert(srcSize == dstSize);
			CmptLifecycleMngr::Instance().MoveConstruct(cmptHash, dstCmpt, srcCmpt);
		}

		// erase
		auto srcMovedIdx = srcArchetype->Erase(srcIdx);
		if (srcMovedIdx != static_cast<size_t>(-1)) {
			auto srcMovedEntityTarget = ai2e.find({ srcArchetype, srcMovedIdx });
			auto srcMovedEntity = srcMovedEntityTarget->second;
			ai2e.erase(srcMovedEntityTarget);
			ai2e[{srcArchetype, srcIdx}] = srcMovedEntity;
			srcMovedEntity->idx = srcIdx;
		}

		ai2e.emplace(std::make_pair(std::make_tuple(dstArchetype, dstIdx), e));

		e->archetype = dstArchetype;
		e->idx = dstIdx;

		/*if (srcArchetype->Size() == 0 && srcArchetype->CmptNum() != 0) {
			ids.erase(srcArchetype->id);
			id2a.erase(srcArchetype->id);
			delete srcArchetype;
		}*/

		return { dstArchetype->At<Cmpts>(dstIdx)... };
	}

	template<typename... Cmpts>
	void ArchetypeMngr::EntityDetach(EntityBase* e) {
		assert(e->archetype->id.IsContain<Cmpts...>());

		Archetype* srcArchetype = e->archetype;
		size_t srcIdx = e->idx;

		auto& srcID = srcArchetype->ID();
		auto dstID = srcID;
		dstID.Remove<Cmpts...>();

		// get dstArchetype
		Archetype* dstArchetype;
		auto target = id2a.find(dstID);
		if (target == id2a.end()) {
			dstArchetype = Archetype::Remove<Cmpts...>(srcArchetype);
			assert(dstID == dstArchetype->ID());
			id2a[dstID] = dstArchetype;
			ids.insert(dstID);
			for (auto& [cmptsHash, archetypes] : cmpts2as) {
				if (dstArchetype->ID().IsContain(cmpts2ids[cmptsHash]))
					archetypes.insert(dstArchetype);
			}
		}
		else
			dstArchetype = target->second;

		// move src to dst
		size_t dstIdx = dstArchetype->RequestBuffer();
		for (auto cmptHash : srcID) {
			auto [srcCmpt, srcSize] = srcArchetype->At(cmptHash, srcIdx);
			if (dstID.IsContain(cmptHash)) {
				auto [dstCmpt, dstSize] = dstArchetype->At(cmptHash, dstIdx);
				assert(srcSize == dstSize);
				CmptLifecycleMngr::Instance().MoveConstruct(cmptHash, dstCmpt, srcCmpt);
			}
		}

		// erase
		auto srcMovedIdx = srcArchetype->Erase(srcIdx);
		if (srcMovedIdx != static_cast<size_t>(-1)) {
			auto srcMovedEntityTarget = ai2e.find({ srcArchetype, srcMovedIdx });
			auto srcMovedEntity = srcMovedEntityTarget->second;
			ai2e.erase(srcMovedEntityTarget);
			ai2e[{srcArchetype, srcIdx}] = srcMovedEntity;
			srcMovedEntity->idx = srcIdx;
		}

		ai2e[{dstArchetype, dstIdx}] = e;

		e->archetype = dstArchetype;
		e->idx = dstIdx;

		/*if (srcArchetype->Size() == 0) {
			ids.erase(srcArchetype->id);
			id2a.erase(srcArchetype->id);
			delete srcArchetype;
		}*/
	}

	template<typename Sys>
	void ArchetypeMngr::GenJob(Job* job, Sys&& sys) {
		using ArgList = FuncTraits_ArgList<std::decay_t<Sys>>;
		using OrderList = CmptTag::GetOrderList_t<ArgList>;
		using TaggedCmptList = CmptTag::RemoveOrders_t<ArgList>;
		return detail::ArchetypeMngr_::GenJob<ArgList, OrderList, TaggedCmptList>::run(job, this, std::forward<Sys>(sys));
	}
}

namespace Ubpa::detail::ArchetypeMngr_ {
	template<typename... Args, typename... Orders, typename... TagedCmpts>
	struct GenJob<TypeList<Args...>, TypeList<Orders...>, TypeList<TagedCmpts...>> {
		static_assert(sizeof...(TagedCmpts) > 0);
		using CmptList = TypeList<CmptTag::RemoveTag_t<TagedCmpts>...>;
		static_assert(IsSet_v<CmptList>, "Componnents must be different");
		template<typename Sys>
		static void run(Job* job, ArchetypeMngr* mngr, Sys&& s) {
			assert(job->empty());
			for (auto archetype : mngr->GetArchetypeWith<CmptTag::RemoveTag_t<TagedCmpts>...>()) {
				auto cmptsTupleVec = archetype->Locate<CmptTag::RemoveTag_t<TagedCmpts>...>();
				size_t num = archetype->Size();
				size_t chunkNum = archetype->ChunkNum();
				size_t chunkCapacity = archetype->ChunkCapacity();

				for (size_t i = 0; i < chunkNum; i++) {
					size_t J = std::min(chunkCapacity, num - (i * chunkCapacity));
					job->emplace([s, cmptsTuple=std::move(cmptsTupleVec[i]), J]() {
						for (size_t j = 0; j < J; j++) {
							s(std::get<Args>(
								std::make_tuple(
									static_cast<TagedCmpts>((std::get<Find_v<CmptList, CmptTag::RemoveTag_t<TagedCmpts>>>(cmptsTuple) + j))...,
									Orders{}...
								))...);
						}
					});
				}
			}
		}
	};
}
