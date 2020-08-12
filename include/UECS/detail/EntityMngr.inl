#pragma once

#include "../CmptTag.h"

#include <UTemplate/Typelist.h>
#include <UTemplate/Func.h>

namespace Ubpa::UECS {
	template<typename... Cmpts>
	Archetype* EntityMngr::GetOrCreateArchetypeOf() {
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"EntityMngr::GetOrCreateArchetypeOf: <Cmpts>... must be different");

		const auto typeset = Archetype::GenCmptTypeSet<Cmpts...>();
		auto target = ts2a.find(typeset);
		if(target != ts2a.end())
			return target->second.get();

		auto archetype = new Archetype(this, TypeList<Cmpts...>{});
		ts2a.emplace(std::move(typeset), std::unique_ptr<Archetype>{ archetype });
		for (auto& [query, archetypes] : queryCache) {
			if (archetype->GetCmptTypeSet().IsMatch(query))
				archetypes.insert(archetype);
		}

		return archetype;
	}

	template<typename... Cmpts>
	std::tuple<Entity, Cmpts*...> EntityMngr::Create() {
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"EntityMngr::Create: <Cmpts>... must be different");
		Archetype* archetype = GetOrCreateArchetypeOf<Cmpts...>();
		size_t entityIndex = RequestEntityFreeEntry();
		EntityInfo& info = entityTable[entityIndex];
		Entity e{ entityIndex, info.version };
		info.archetype = archetype;
		auto [idxInArchetype, cmpts] = archetype->Create<Cmpts...>(e);
		info.idxInArchetype = idxInArchetype;
		return { e, std::get<Cmpts*>(cmpts)... };
	}

	template<typename... Cmpts>
	void EntityMngr::AttachWithoutInit(Entity e) {
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"EntityMngr::AttachWithoutInit: <Cmpts>... must be different");
		if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

		auto& info = entityTable[e.Idx()];
		Archetype* srcArchetype = info.archetype;
		size_t srcIdxInArchetype = info.idxInArchetype;

		const auto& srcCmptTypeSet = srcArchetype->GetCmptTypeSet();
		auto dstCmptTypeSet = srcCmptTypeSet;
		dstCmptTypeSet.data.insert(CmptType::Of<Cmpts>...);

		// get dstArchetype
		Archetype* dstArchetype;
		auto target = ts2a.find(dstCmptTypeSet);
		if (target == ts2a.end()) {
			dstArchetype = Archetype::Add<Cmpts...>(srcArchetype);
			assert(dstCmptTypeSet == dstArchetype->GetCmptTypeSet());
			for (auto& [query, archetypes] : queryCache) {
				if (dstCmptTypeSet.IsMatch(query))
					archetypes.insert(dstArchetype);
			}
			ts2a.emplace(std::move(dstCmptTypeSet), std::unique_ptr<Archetype>{ dstArchetype });
		}
		else
			dstArchetype = target->second.get();

		if (dstArchetype == srcArchetype)
			return;

		// move src to dst
		size_t dstIdxInArchetype = dstArchetype->RequestBuffer();

		auto srcCmptTraits = srcArchetype->GetRTSCmptTraits();
		for (const auto& type : srcCmptTypeSet.data) {
			auto srcCmpt = srcArchetype->At(type, srcIdxInArchetype);
			auto dstCmpt = dstArchetype->At(type, dstIdxInArchetype);
			srcCmptTraits.MoveConstruct(type, dstCmpt, srcCmpt);
		}

		// erase
		auto srcMovedEntityIndex = srcArchetype->Erase(srcIdxInArchetype);
		if (srcMovedEntityIndex != size_t_invalid)
			entityTable[srcMovedEntityIndex].idxInArchetype = srcIdxInArchetype;

		info.archetype = dstArchetype;
		info.idxInArchetype = dstIdxInArchetype;
	}

	template<typename... Cmpts>
	std::tuple<Cmpts*...> EntityMngr::Attach(Entity e) {
		static_assert((std::is_default_constructible_v<Cmpts> &&...),
			"EntityMngr::Attach: <Cmpts> isn't default constructible");

		using CmptList = TypeList<Cmpts...>;
		const auto& cmptTypes = entityTable[e.Idx()].archetype->GetCmptTypeSet();
		std::array needAttach = { !cmptTypes.Contains(CmptType::Of<Cmpts>)... };
		AttachWithoutInit<Cmpts...>(e);
		const auto& new_info = entityTable[e.Idx()];
		std::tuple cmpts{ new_info.archetype->At<Cmpts>(new_info.idxInArchetype)... };
		((std::get<Find_v<CmptList, Cmpts>>(needAttach) ? new(std::get<Cmpts*>(cmpts))Cmpts : nullptr), ...);

		return cmpts;
	}

	template<typename Cmpt, typename... Args>
	Cmpt* EntityMngr::Emplace(Entity e, Args&&... args) {
		static_assert(std::is_constructible_v<Cmpt, Args...>
			|| is_list_initializable_v<Cmpt, Args...>,
			"EntityMngr::Emplace: <Cmpt> isn't constructible/list_initializable with Args...");

		if (!Have(e, CmptType::Of<Cmpt>)) {
			AttachWithoutInit<Cmpt>(e);
			return new(Get<Cmpt>(e))Cmpt{ std::forward<Args>(args)... };
		}
		else
			return Get<Cmpt>(e);
	}

	inline bool EntityMngr::Have(Entity e, CmptType type) const {
		assert(!type.Is<Entity>());
		if (!Exist(e)) throw std::invalid_argument("EntityMngr::Have: Entity is invalid");
		return entityTable[e.Idx()].archetype->GetCmptTypeSet().Contains(type);
	}

	template<typename Cmpt>
	Cmpt* EntityMngr::Get(Entity e) const {
		static_assert(!std::is_same_v<Cmpt, Entity>, "EntityMngr::Get: <Cmpt> != Entity");
		return reinterpret_cast<Cmpt*>(Get(e, CmptType::Of<Cmpt>).Ptr());
	}

	inline CmptPtr EntityMngr::Get(Entity e, CmptType type) const {
		assert(!type.Is<Entity>());
		if (!Exist(e)) throw std::invalid_argument("EntityMngr::Get: Entity is invalid");
		const auto& info = entityTable[e.Idx()];
		return { type, info.archetype->At(type, info.idxInArchetype) };
	}

	inline bool EntityMngr::Exist(Entity e) const {
		return e.Idx() < entityTable.size() && e.Version() == entityTable[e.Idx()].version;
	}
}
