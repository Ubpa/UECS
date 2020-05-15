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

	template<typename... CmptTypes, typename>
	Archetype* EntityMngr::GetOrCreateArchetypeOf(CmptTypes... types) {
		static_assert((std::is_same_v<CmptTypes, CmptType> &&...));
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		return GetOrCreateArchetypeOf(typeArr.data(), typeArr.size());
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

	template<typename... CmptTypes, typename>
	Entity EntityMngr::CreateEntity(CmptTypes... types) {
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		return CreateEntity(typeArr.data(), typeArr.size());
	}

	template<typename... CmptTypes, typename>
	void EntityMngr::AttachWithoutInit(Entity e, CmptTypes... types) {
		static_assert(sizeof...(CmptTypes) > 0,
			"EntityMngr::AttachWithoutInit: !types.empty()");
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		return AttachWithoutInit(e, typeArr.data(), typeArr.size());
	}

	template<typename... Cmpts>
	std::tuple<Cmpts*...> EntityMngr::Attach(Entity e) {
		static_assert((std::is_constructible_v<Cmpts> &&...),
			"EntityMngr::Attach: <Cmpts> isn't constructible");
		if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

		using CmptList = TypeList<Cmpts...>;
		std::array<bool, sizeof...(Cmpts)> needAttach
			= { entityTable[e.Idx()].archetype->GetCmptTypeSet().IsNotContain<Cmpts>()... };
		AttachWithoutInit(e, CmptType::Of<Cmpts>()...);
		const auto& new_info = entityTable[e.Idx()];
		std::tuple<Cmpts*...> cmpts{ new_info.archetype->At<Cmpts>(new_info.idxInArchetype)... };
		((std::get<Find_v<CmptList, Cmpts>>(needAttach) ? new(std::get<Cmpts*>(cmpts))Cmpts : nullptr), ...);

		return cmpts;
	}

	template<typename... CmptTypes, typename>
	std::array<CmptPtr, sizeof...(CmptTypes)> EntityMngr::Attach(Entity e, CmptTypes... types) {
		if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

		constexpr size_t N = sizeof...(types);

		std::array<CmptType, N> typeArr = { types... };
		auto srcArchetype = entityTable[e.Idx()].archetype;
		AttachWithoutInit(e, types...);
		const auto& new_info = entityTable[e.Idx()];
		std::array<CmptPtr, sizeof...(CmptTypes)> cmptPtrArr
			= { CmptPtr{types, std::get<void*>(new_info.archetype->At(types, new_info.idxInArchetype))}... };
		const auto& rtdct = RTDCmptTraits::Instance();
		for (size_t i = 0; i < N; i++) {
			auto type = typeArr[i];
			if (srcArchetype->GetCmptTypeSet().IsContain(type))
				continue;
			auto target = rtdct.default_constructors.find(type);
			if (target == rtdct.default_constructors.end())
				continue;
			
			target->second(cmptPtrArr[i].Ptr());
		}

		return cmptPtrArr;
	}

	template<typename Cmpt, typename... Args>
	Cmpt* EntityMngr::Emplace(Entity e, Args&&... args) {
		static_assert(std::is_constructible_v<Cmpt, Args...>
			|| detail::EntityMngr_::IsAggregatable_v<Cmpt, Args...>,
			"EntityMngr::Emplace: <Cmpt> isn't constructible/aggregatable with <Args...>");
		if (!Exist(e)) throw std::invalid_argument("EntityMngr::Emplace: Entity is invalid");

		bool needAttach = entityTable[e.Idx()].archetype->GetCmptTypeSet().IsNotContain<Cmpt>();
		if (needAttach) {
			AttachWithoutInit(e, CmptType::Of<Cmpt>());
			const auto& info = entityTable[e.Idx()];
			Cmpt* cmpt = info.archetype->At<Cmpt>(info.idxInArchetype);
			return new(cmpt)Cmpt{ std::forward<Args>(args)... };
		}
		else {
			const auto& info = entityTable[e.Idx()];
			return info.archetype->At<Cmpt>(info.idxInArchetype);
		}
	}

	template<typename... Cmpts>
	void EntityMngr::Detach(Entity e) {
		static_assert(sizeof...(Cmpts) > 0, "EntityMngr::Detach: sizeof...(<Cmpts>) > 0");
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"EntityMngr::Detach: <Cmpts> must be different");
		Detach(e, CmptType::Of<Cmpts>()...);
	}

	template<typename... CmptTypes, typename>
	void EntityMngr::Detach(Entity e, CmptTypes... types) {
		static_assert((std::is_same_v<CmptTypes, CmptType> &&...));
		const std::array<CmptType, sizeof...(CmptTypes)> typeArr{ types... };
		return Detach(e, typeArr.data(), typeArr.size());
	}

	template<typename Cmpt>
	bool EntityMngr::Have(Entity e) const {
		static_assert(!std::is_same_v<Cmpt, Entity>, "EntityMngr::Have: <Cmpt> != Entity");
		return Have(e, CmptType::Of<Cmpt>());
	}

	inline bool EntityMngr::Have(Entity e, CmptType type) const {
		if (!Exist(e)) throw std::invalid_argument("EntityMngr::Have: Entity is invalid");
		return entityTable[e.Idx()].archetype->GetCmptTypeSet().IsContain(type);
	}

	template<typename Cmpt>
	Cmpt* EntityMngr::Get(Entity e) const {
		static_assert(!std::is_same_v<Cmpt, Entity>, "EntityMngr::Get: <Cmpt> != Entity");
		if (!Exist(e)) throw std::invalid_argument("EntityMngr::Get: Entity is invalid");
		const auto& info = entityTable[e.Idx()];
		return info.archetype->At<Cmpt>(info.idxInArchetype);
	}

	inline CmptPtr EntityMngr::Get(Entity e, CmptType type) const {
		if (!Exist(e)) throw std::invalid_argument("EntityMngr::Get: Entity is invalid");
		const auto& info = entityTable[e.Idx()];
		return { type, std::get<void*>(info.archetype->At(type, info.idxInArchetype)) };
	}
}
