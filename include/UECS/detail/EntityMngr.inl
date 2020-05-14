#pragma once

#include "../CmptTag.h"

#include <UTemplate/Typelist.h>
#include <UTemplate/Func.h>
#include <UTemplate/Concept.h>

namespace Ubpa::detail::EntityMngr_ {
	template<typename Cmpt, typename... Ts>
	Concept(IsAggregatableHelper, Cmpt{ std::declval<Ts>()... });
	template<typename Cmpt, typename... Ts>
	struct IsAggregatable : IValue<bool, std::is_aggregate_v<Cmpt> && Require<IsAggregatableHelper, Cmpt, Ts...>> {};
	template<typename Cmpt, typename... Ts>
	static constexpr bool IsAggregatable_v = IsAggregatable<Cmpt, Ts...>::value;
}

#include <stdexcept>

namespace Ubpa {
	template<typename... Cmpts>
	Archetype* EntityMngr::GetOrCreateArchetypeOf() {
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"EntityMngr::GetOrCreateArchetypeOf: <Cmpts> must be different");

		constexpr size_t hashcode = Archetype::HashCode<Cmpts...>();
		auto target = h2a.find(hashcode);
		if(target != h2a.end())
			return target->second;

		auto archetype = new Archetype(TypeList<Cmpts...>{});
		h2a[hashcode] = archetype;
		for (auto& [query, archetypes] : queryCache) {
			if (archetype->GetCmptTypeSet().IsMatch(query))
				archetypes.insert(archetype);
		}

		return archetype;
	}

	template<typename... Cmpts>
	std::tuple<Entity, Cmpts*...> EntityMngr::CreateEntity() {
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"EntityMngr::CreateEntity: <Cmpts> must be different");
		Archetype* archetype = GetOrCreateArchetypeOf<Cmpts...>();
		size_t entityIndex = RequestEntityFreeEntry();
		EntityInfo& info = entityTable[entityIndex];
		Entity e{ entityIndex, info.version };
		info.archetype = archetype;
		auto [idxInArchetype, cmpts] = archetype->CreateEntity<Cmpts...>(e);
		info.idxInArchetype = idxInArchetype;
		return { e, std::get<Cmpts*>(cmpts)... };
	}

	template<typename... Cmpts>
	std::tuple<std::array<bool, sizeof...(Cmpts)>, std::tuple<Cmpts*...>>  EntityMngr::AttachWithoutInit(Entity e) {
		static_assert(sizeof...(Cmpts) > 0,
			"EntityMngr::AttachWithoutInit: sizeof...(<Cmpts>) > 0");
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"EntityMngr::AttachWithoutInit: <Cmpts> must be different");
		if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

		auto& info = entityTable[e.Idx()];
		Archetype* srcArchetype = info.archetype;
		size_t srcIdxInArchetype = info.idxInArchetype;

		const auto& srcCmptTypeSet = srcArchetype->GetCmptTypeSet();
		auto dstCmptTypeSet = srcCmptTypeSet;
		dstCmptTypeSet.Insert<Cmpts...>();
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

		if (dstArchetype == srcArchetype)
			return { std::array<bool, sizeof...(Cmpts)>{false}, {srcArchetype->At<Cmpts>(srcIdxInArchetype)...} };

		// move src to dst
		size_t dstIdxInArchetype = dstArchetype->RequestBuffer();

		auto srcCmptTraits = srcArchetype->GetRuntimeCmptTraits();
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

		return { {srcArchetype->GetCmptTypeSet().IsNotContain<Cmpts>()...}, {dstArchetype->At<Cmpts>(dstIdxInArchetype)...} };
	}

	template<typename... Cmpts>
	std::tuple<Cmpts*...> EntityMngr::Attach(Entity e) {
		static_assert((std::is_constructible_v<Cmpts> &&...),
			"EntityMngr::Attach: <Cmpts> isn't constructible");
		using CmptList = TypeList<Cmpts...>;
		auto [success, cmpts] = AttachWithoutInit<Cmpts...>(e);

		return { (std::get<Find_v<CmptList,Cmpts>>(success) ? new(std::get<Cmpts*>(cmpts))Cmpts : std::get<Cmpts*>(cmpts))... };
	}

	template<typename Cmpt, typename... Args>
	Cmpt* EntityMngr::Emplace(Entity e, Args&&... args) {
		static_assert(std::is_constructible_v<Cmpt, Args...> || detail::EntityMngr_::IsAggregatable_v<Cmpt, Args...>,
			"EntityMngr::Emplace: <Cmpt> isn't constructible/aggregatable with <Args...>");
		auto [success, cmpt] = AttachWithoutInit<Cmpt>(e);
		return std::get<0>(success) ? new(std::get<Cmpt*>(cmpt))Cmpt{ std::forward<Args>(args)... } : std::get<Cmpt*>(cmpt);
	}

	template<typename... Cmpts>
	void EntityMngr::Detach(Entity e) {
		static_assert(sizeof...(Cmpts) > 0, "EntityMngr::Detach: sizeof...(<Cmpts>) > 0");
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"EntityMngr::Detach: <Cmpts> must be different");
		if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

		auto& info = entityTable[e.Idx()];
		Archetype* srcArchetype = info.archetype;
		size_t srcIdxInArchetype = info.idxInArchetype;

		const auto& srcCmptTypeSet = srcArchetype->GetCmptTypeSet();
		auto dstCmptTypeSet = srcCmptTypeSet;
		dstCmptTypeSet.Erase<Cmpts...>();
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

		if (dstArchetype == srcArchetype)
			return;

		// move src to dst
		size_t dstIdxInArchetype = dstArchetype->RequestBuffer();
		auto srcCmptTraits = srcArchetype->GetRuntimeCmptTraits();
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

	template<typename Cmpt>
	bool EntityMngr::Have(Entity e) const {
		static_assert(!std::is_same_v<Cmpt, Entity>, "EntityMngr::Have: <Cmpt> != Entity");
		if (!Exist(e)) throw std::invalid_argument("Entity is invalid");
		return entityTable[e.Idx()].archetype->GetCmptTypeSet().IsContain<Cmpt>();
	}

	template<typename Cmpt>
	Cmpt* EntityMngr::Get(Entity e) const {
		static_assert(!std::is_same_v<Cmpt, Entity>, "EntityMngr::Get: <Cmpt> != Entity");
		if (!Exist(e)) throw std::invalid_argument("Entity is invalid");
		const auto& info = entityTable[e.Idx()];
		return info.archetype->At<Cmpt>(info.idxInArchetype);
	}
}
