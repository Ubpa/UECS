#pragma once

#include "../CmptTag.h"

#include <UTemplate/Typelist.h>
#include <UTemplate/Func.h>

namespace Ubpa::detail::EntityMngr_ {
	/*template<typename ArgList, typename TaggedCmptList, typename OtherArgList>
	struct GenJob;*/
}

namespace Ubpa {
	template<typename... Cmpts>
	inline Archetype* EntityMngr::GetOrCreateArchetypeOf() {
		static_assert(IsSet_v<TypeList<Cmpts...>>,
			"EntityMngr::GetOrCreateArchetypeOf: <Cmpts> must be different");

		auto types = CmptTypeSet(TypeList<Cmpts...>{});
		constexpr size_t typesHashCode = CmptTypeSet::HashCodeOf<Cmpts...>();
		auto target = h2a.find(typesHashCode);
		if(target != h2a.end())
			return target->second;

		auto archetype = new Archetype(this, TypeList<Cmpts...>{});
		h2a[typesHashCode] = archetype;
		for (auto& [query, archetypes] : queryCache) {
			if (types.IsMatch(query))
				archetypes.insert(archetype);
		}
		return archetype;
	}

	template<typename... Cmpts>
	const std::tuple<EntityData*, Cmpts*...> EntityMngr::CreateEntity() {
		Archetype* archetype = GetOrCreateArchetypeOf<Cmpts...>();
		auto [idx, cmpts] = archetype->CreateEntity<Cmpts...>();

		auto entity = entityPool.Request(archetype, idx);
		ai2e[{archetype,idx}] = entity;

		using CmptList = TypeList<Cmpts...>;
		return { entity, std::get<Cmpts*>(cmpts)... };
	}

	template<typename... Cmpts>
	const std::tuple<Cmpts*...> EntityMngr::EntityAttachWithoutInit(EntityData* e) {
		static_assert(sizeof...(Cmpts) > 0, "EntityMngr::EntityAttach: sizeof...(<Cmpts>) > 0");
		static_assert(IsSet_v<TypeList<Cmpts...>>, "EntityMngr::EntityAttach: <Cmpts> must be different");
		assert((e->archetype->cmptTypeSet.IsNotContain<Cmpts>() &&...));

		Archetype* srcArchetype = e->archetype;
		size_t srcIdx = e->idx;

		auto& srcCmptTypeSet = srcArchetype->GetCmptTypeSet();
		auto dstCmptTypeSet = srcCmptTypeSet;
		dstCmptTypeSet.Add<Cmpts...>();
		size_t dstCmptTypeSetHashCode = dstCmptTypeSet.HashCode();

		// get dstArchetype
		Archetype* dstArchetype;
		auto target = h2a.find(dstCmptTypeSetHashCode);
		if (target == h2a.end()) {
			dstArchetype = Archetype::Add<Cmpts...>(srcArchetype);
			assert(dstCmptTypeSet == dstArchetype->GetCmptTypeSet());
			h2a[dstCmptTypeSetHashCode] = dstArchetype;
			for (auto& [query, archetypes] : queryCache) {
				if (dstCmptTypeSet.IsMatch(query))
					archetypes.insert(dstArchetype);
			}
		}
		else
			dstArchetype = target->second;

		// move src to dst
		size_t dstIdx = dstArchetype->RequestBuffer();

		for (auto cmptID : srcCmptTypeSet) {
			auto [srcCmpt, srcSize] = srcArchetype->At(cmptID, srcIdx);
			auto [dstCmpt, dstSize] = dstArchetype->At(cmptID, dstIdx);
			assert(srcSize == dstSize);
			RuntimeCmptTraits::Instance().MoveConstruct(cmptID, srcSize, dstCmpt, srcCmpt);
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
		else
			ai2e.erase({ srcArchetype, srcIdx });

		ai2e.emplace(std::make_pair(std::make_tuple(dstArchetype, dstIdx), e));

		e->archetype = dstArchetype;
		e->idx = dstIdx;

		/*if (srcArchetype->Size() == 0 && srcArchetype->CmptNum() != 0) {
			h2a.erase(srcArchetype->cmptTypeSet);
			delete srcArchetype;
		}*/

		return { dstArchetype->At<Cmpts>(dstIdx)... };
	}

	template<typename... Cmpts>
	const std::tuple<Cmpts*...> EntityMngr::EntityAttach(EntityData* e) {
		static_assert((std::is_constructible_v<Cmpts> &&...),
			"EntityMngr::EntityAttach: <Cmpts> isn't constructible");

		auto cmpts = EntityAttachWithoutInit<Cmpts...>(e);

		return { new(std::get<Cmpts*>(cmpts))Cmpts... };
	}

	template<typename Cmpt, typename... Args>
	Cmpt* EntityMngr::EntityAssignAttach(EntityData* e, Args&&... args) {
		static_assert(std::is_constructible_v<Cmpt, Args...>,
			"EntityMngr::EntityAssignAttach: <Cmpt> isn't constructible with <Args...>");
		auto [cmpt] = EntityAttachWithoutInit<Cmpt>(e);
		return new(cmpt)Cmpt{ std::forward<Args>(args)... };
	}

	template<typename... Cmpts>
	void EntityMngr::EntityDetach(EntityData* e) {
		static_assert(sizeof...(Cmpts) > 0, "EntityMngr::EntityAttach: sizeof...(<Cmpts>) > 0");
		static_assert(IsSet_v<TypeList<Cmpts...>>, "EntityMngr::EntityAttach: <Cmpts> must be different");

		assert((e->archetype->cmptTypeSet.IsContain<Cmpts>() &&...));

		Archetype* srcArchetype = e->archetype;
		size_t srcIdx = e->idx;

		auto& srcCmptTypeSet = srcArchetype->GetCmptTypeSet();
		auto dstCmptTypeSet = srcCmptTypeSet;
		dstCmptTypeSet.Remove<Cmpts...>();
		size_t dstCmptTypeSetHashCode = dstCmptTypeSet.HashCode();

		// get dstArchetype
		Archetype* dstArchetype;
		auto target = h2a.find(dstCmptTypeSetHashCode);
		if (target == h2a.end()) {
			dstArchetype = Archetype::Remove<Cmpts...>(srcArchetype);
			assert(dstCmptTypeSet == dstArchetype->GetCmptTypeSet());
			h2a[dstCmptTypeSetHashCode] = dstArchetype;
			for (auto& [query, archetypes] : queryCache) {
				if (dstCmptTypeSet.IsMatch(query))
					archetypes.insert(dstArchetype);
			}
		}
		else
			dstArchetype = target->second;

		// move src to dst
		size_t dstIdx = dstArchetype->RequestBuffer();
		for (auto cmptID : srcCmptTypeSet) {
			if (dstCmptTypeSet.IsContain(cmptID)) {
				auto [srcCmpt, srcSize] = srcArchetype->At(cmptID, srcIdx);
				auto [dstCmpt, dstSize] = dstArchetype->At(cmptID, dstIdx);
				assert(srcSize == dstSize);
				RuntimeCmptTraits::Instance().MoveConstruct(cmptID, srcSize, dstCmpt, srcCmpt);
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
		else
			ai2e.erase({ srcArchetype, srcIdx });

		ai2e[{dstArchetype, dstIdx}] = e;

		e->archetype = dstArchetype;
		e->idx = dstIdx;

		/*if (srcArchetype->Size() == 0) {
			h2a.erase(srcArchetype->cmptTypeSet);
			delete srcArchetype;
		}*/
	}

	/*template<typename Sys>
	void EntityMngr::GenJob(Job* job, Sys&& sys) const {
		using ArgList = FuncTraits_ArgList<std::decay_t<Sys>>;
		using TaggedCmptList = CmptTag::GetTimePointList_t<ArgList>;
		using OtherArgList = CmptTag::RemoveTimePoint_t<ArgList>;
		return detail::EntityMngr_::GenJob<ArgList, TaggedCmptList, OtherArgList>::run(job, this, std::forward<Sys>(sys));
	}*/

	template<typename... Cmpts>
	std::vector<CmptType> EntityMngr::TypeListToTypeVec(TypeList<Cmpts...>) {
		return { CmptType::Of<Cmpts>()... };
	}
}

namespace Ubpa::detail::EntityMngr_ {
	/*template<typename... Args, typename... TaggedCmpts, typename... OtherArgs>
	struct GenJob<TypeList<Args...>, TypeList<TaggedCmpts...>, TypeList<OtherArgs...>> {
		static_assert(sizeof...(TaggedCmpts) > 0);
		using CmptList = TypeList<CmptTag::RemoveTag_t<TaggedCmpts>...>;
		using ConcatedAll = CmptTag::ConcatedAll_t<TypeList<Args...>>;
		using ConcatedAny = CmptTag::ConcatedAny_t<TypeList<Args...>>;
		using ConcatedNone = CmptTag::ConcatedNone_t<TypeList<Args...>>;
		static_assert(IsSet_v<CmptList>, "Componnents must be different");
		template<typename Sys>
		static void run(Job* job, const EntityMngr* mngr, Sys&& s) {
			assert(job->empty());
			for (const Archetype* archetype : mngr->QueryArchetypes<ConcatedAll, ConcatedAny, ConcatedNone, CmptList>()) {
				auto cmptsTupleVec = archetype->Locate<CmptTag::RemoveTag_t<TaggedCmpts>...>();
				size_t num = archetype->EntityNum();
				size_t chunkNum = archetype->ChunkNum();
				size_t chunkCapacity = archetype->ChunkCapacity();

				for (size_t i = 0; i < chunkNum; i++) {
					size_t J = std::min(chunkCapacity, num - (i * chunkCapacity));
					job->emplace([s, cmptsTuple=std::move(cmptsTupleVec[i]), J]() {
						for (size_t j = 0; j < J; j++) {
							s(std::get<Args>(
								std::make_tuple(
									static_cast<TaggedCmpts>((std::get<Find_v<CmptList, CmptTag::RemoveTag_t<TaggedCmpts>>>(cmptsTuple) + j))...,
									OtherArgs{}...
								))...);
						}
					});
				}
			}
		}
	};*/
}
