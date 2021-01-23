#pragma once

#include "../CmptTag.h"

#include <UTemplate/TypeList.h>
#include <UTemplate/Func.h>

namespace Ubpa::UECS {
	template<typename... Cmpts>
	Archetype* EntityMngr::GetOrCreateArchetypeOf() {
		const auto typeset = Archetype::GenTypeIDSet<Cmpts...>();
		auto target = ts2a.find(typeset);
		if(target != ts2a.end())
			return target->second.get();

		auto archetype = new Archetype(std::pmr::polymorphic_allocator<Chunk>{rsrc.get()}, TypeList<Cmpts...>{});
		ts2a.emplace(std::move(typeset), std::unique_ptr<Archetype>{ archetype });
		for (auto& [query, archetypes] : queryCache) {
			if (archetype->GetTypeIDSet().IsMatch(query))
				archetypes.insert(archetype);
		}

		return archetype;
	}

	template<typename... Cmpts>
	std::tuple<Entity, Cmpts*...> EntityMngr::Create() {
		Archetype* archetype = GetOrCreateArchetypeOf<Cmpts...>();
		std::size_t entityIndex = RequestEntityFreeEntry();
		EntityInfo& info = entityTable[entityIndex];
		Entity e{ entityIndex, info.version };
		info.archetype = archetype;
		auto [idxInArchetype, cmpts] = archetype->Create<Cmpts...>(e);
		info.idxInArchetype = idxInArchetype;
		return { e, std::get<Cmpts*>(cmpts)... };
	}

	template<typename... Cmpts>
	Archetype* EntityMngr::AttachWithoutInit(Entity e) {
		if (!Exist(e)) throw std::invalid_argument("Entity is invalid");

		auto& info = entityTable[e.Idx()];
		Archetype* srcArchetype = info.archetype;

		const auto& srcTypeIDSet = srcArchetype->GetTypeIDSet();
		auto dstTypeIDSet = srcTypeIDSet;
		(dstTypeIDSet.data.insert(TypeID_of<Cmpts>), ...);

		// get dstArchetype
		Archetype* dstArchetype;
		auto target = ts2a.find(dstTypeIDSet);
		if (target == ts2a.end()) {
			dstArchetype = Archetype::Add<Cmpts...>(srcArchetype);
			assert(dstTypeIDSet == dstArchetype->GetTypeIDSet());
			for (auto& [query, archetypes] : queryCache) {
				if (dstTypeIDSet.IsMatch(query))
					archetypes.insert(dstArchetype);
			}
			ts2a.emplace(std::move(dstTypeIDSet), std::unique_ptr<Archetype>{ dstArchetype });
		}
		else {
			dstArchetype = target->second.get();
			if (dstArchetype == srcArchetype)
				return srcArchetype;
		}

		// move src to dst
		std::size_t srcIdxInArchetype = info.idxInArchetype;
		std::size_t dstIdxInArchetype = dstArchetype->RequestBuffer();

		auto srcCmptTraits = srcArchetype->GetArchetypeCmptTraits();
		for (const auto& type : srcTypeIDSet.data) {
			auto srcCmpt = srcArchetype->At(type, srcIdxInArchetype);
			auto dstCmpt = dstArchetype->At(type, dstIdxInArchetype);
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

	template<typename... Cmpts>
	std::tuple<Cmpts*...> EntityMngr::Attach(Entity e) {
		using CmptList = TypeList<Cmpts...>;
		auto origArchetype = AttachWithoutInit<Cmpts...>(e);
		const auto& cmptTypes = origArchetype->GetTypeIDSet();
		std::array needAttach = { !cmptTypes.Contains(TypeID_of<Cmpts>)... };
		const auto& new_info = entityTable[e.Idx()];
		std::tuple cmpts{ new_info.archetype->At<Cmpts>(new_info.idxInArchetype)... };
		((std::get<Find_v<CmptList, Cmpts>>(needAttach) ? new(std::get<Cmpts*>(cmpts))Cmpts : nullptr), ...);

		return cmpts;
	}

	template<typename... Cmpts>
	void EntityMngr::Detach(Entity e) {
		constexpr std::array types{ TypeID_of<Cmpts>... };
		Detach(e, types);
	}

	template<typename Cmpt>
	bool EntityMngr::Have(Entity e) const {
		return Have(e, TypeID_of<Cmpt>);
	}

	template<typename Cmpt, typename... Args>
	Cmpt* EntityMngr::Emplace(Entity e, Args&&... args) {
		static_assert(std::is_constructible_v<Cmpt, Args...>
			|| is_list_initializable_v<Cmpt, Args...>,
			"<Cmpt> isn't constructible/list_initializable with Args...");

		assert(!Have(e, TypeID_of<Cmpt>));
		AttachWithoutInit<Cmpt>(e);
		return new(Get<Cmpt>(e))Cmpt{ std::forward<Args>(args)... };
	}

	inline bool EntityMngr::Have(Entity e, TypeID type) const {
		assert(!type.Is<Entity>());
		if (!Exist(e)) throw std::invalid_argument("EntityMngr::Have: Entity is invalid");
		return entityTable[e.Idx()].archetype->GetTypeIDSet().Contains(type);
	}

	template<typename Cmpt>
	Cmpt* EntityMngr::Get(Entity e) const {
		static_assert(!std::is_same_v<Cmpt, Entity>, "EntityMngr::Get: <Cmpt> != Entity");
		return reinterpret_cast<Cmpt*>(Get(e, TypeID_of<Cmpt>).Ptr());
	}

	inline CmptPtr EntityMngr::Get(Entity e, TypeID type) const {
		assert(!type.Is<Entity>());
		if (!Exist(e)) throw std::invalid_argument("EntityMngr::Get: Entity is invalid");
		const auto& info = entityTable[e.Idx()];
		return { type, info.archetype->At(type, info.idxInArchetype) };
	}

	inline bool EntityMngr::Exist(Entity e) const noexcept {
		return e.Idx() < entityTable.size() && e.Version() == entityTable[e.Idx()].version;
	}

	template<typename Cmpt>
	std::vector<Cmpt*> EntityMngr::GetCmptArray(const ArchetypeFilter& filter) const {
		constexpr auto type = TypeID_of<Cmpt>;
		assert(filter.all.find(type) != filter.all.end()); // transparent less

		std::vector<Cmpt*> rst;

		const auto& archetypes = QueryArchetypes(EntityQuery{filter});
		std::size_t num = 0;
		for (const auto& archetype : archetypes)
			num += archetype->EntityNum();

		rst.resize(num);
		std::size_t idx = 0;
		for (auto archetype : archetypes) {
			/*for (std::size_t i = 0; i < archetype->EntityNum(); i++)
				rst[idx++] = archetype->At<TypeID>(i);*/

			// speed up

			std::size_t offset = archetype->Offsetof(type);
			for (std::size_t c = 0; c < archetype->chunks.size(); c++) {
				auto buffer = archetype->chunks[c]->Data();
				auto beg = buffer + offset;
				std::size_t chunkSize = archetype->EntityNumOfChunk(c);
				for (std::size_t i = 0; i < chunkSize; i++)
					rst[idx++] = reinterpret_cast<Cmpt*>(beg + i * sizeof(Cmpt));
			}
		}

		return rst;
	}
}
