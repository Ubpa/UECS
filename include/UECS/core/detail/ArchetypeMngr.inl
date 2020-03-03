#pragma once

namespace Ubpa {
	template<typename... Cmpts>
	inline Archetype* ArchetypeMngr::GetOrCreateArchetypeOf() {
		auto id = Archetype::ID(TypeList<Cmpts...>{});
		auto target = id2a.find(id);
		if (target == id2a.end()) {
			auto archetype = new Archetype(this, TypeList<Cmpts...>{});
			id2a[id] = archetype;
			ids.insert(archetype->GetID());
			return archetype;
		}
		else
			return target->second;
	}

	template<typename... Cmpts>
	const std::tuple<EntityData*, Cmpts*...> ArchetypeMngr::CreateEntity() {
		Archetype* archetype = GetOrCreateArchetypeOf<Cmpts...>();
		size_t idx = archetype->CreateEntity<Cmpts...>();

		auto entity = entityPool.request();
		entity->archetype() = archetype;
		entity->idx() = idx;
		d2p[*entity] = entity;

		return { entity, archetype->At<Cmpts>(idx)... };
	}

	template<typename... Cmpts>
	const std::vector<Archetype*> ArchetypeMngr::GetArchetypeWith() {
		std::vector<Archetype*> rst;
		for (auto& p : id2a) {
			if (p.second->IsContain<Cmpts...>())
				rst.push_back(p.second);
		}
		return rst;
	}

	template<typename... Cmpts>
	const std::tuple<Cmpts*...> ArchetypeMngr::EntityAttach(EntityData* e) {
		assert(!e->archetype()->id.IsContain<Cmpts...>());

		Archetype* srcArchetype = e->archetype();
		size_t srcIdx = e->idx();

		auto& srcID = srcArchetype->GetID();
		auto dstID = srcID;
		dstID.Add<Cmpts...>();

		Archetype* dstArchetype;
		auto target = id2a.find(dstID);
		if (target == id2a.end()) {
			dstArchetype = Archetype::Add<Cmpts...>::From(srcArchetype);
			assert(dstID == dstArchetype->GetID());
			id2a[dstID] = dstArchetype;
			ids.insert(dstID);
		}
		else
			dstArchetype = target->second;

		size_t dstIdx = dstArchetype->CreateEntity();
		for (auto cmptHash : srcID) {
			auto srcCmpt = srcArchetype->At(cmptHash, srcIdx);
			auto dstCmpt = dstArchetype->At(cmptHash, dstIdx);
			size_t size = std::get<1>(srcCmpt);
			assert(size == std::get<1>(dstCmpt));
			memcpy(std::get<0>(dstCmpt), std::get<0>(srcCmpt), size);
		}

		size_t srcMovedIdx = srcArchetype->Erase(srcIdx);
		if (srcMovedIdx != static_cast<size_t>(-1)) {
			auto srcMovedEntityTarget = d2p.find({ srcArchetype, srcMovedIdx });
			auto srcMovedEntity = srcMovedEntityTarget->second;
			d2p.erase(srcMovedEntityTarget);
			d2p[{srcArchetype, srcIdx}] = srcMovedEntity;
			srcMovedEntity->idx() = srcMovedIdx;
		}

		d2p[{dstArchetype, dstIdx}] = e;

		e->archetype() = dstArchetype;
		e->idx() = dstIdx;

		if (srcArchetype->Size() == 0 && srcArchetype->CmptNum() != 0) {
			ids.erase(srcArchetype->id);
			id2a.erase(srcArchetype->id);
			delete srcArchetype;
		}

		return { dstArchetype->At<Cmpts>(dstIdx)... };
	}

	template<typename... Cmpts>
	void ArchetypeMngr::EntityDetach(EntityData* e) {
		assert(e->archetype()->id.IsContain<Cmpts...>());

		Archetype* srcArchetype = e->archetype();
		size_t srcIdx = e->idx();

		auto& srcID = srcArchetype->GetID();
		auto dstID = srcID;
		dstID.Remove<Cmpts...>();

		Archetype* dstArchetype;
		auto target = id2a.find(dstID);
		if (target == id2a.end()) {
			dstArchetype = Archetype::Remove<Cmpts...>::From(srcArchetype);
			assert(dstID == dstArchetype->GetID());
			id2a[dstID] = dstArchetype;
			ids.insert(dstID);
		}
		else
			dstArchetype = target->second;

		size_t dstIdx = dstArchetype->CreateEntity();
		for (auto cmptHash : dstID) {
			auto srcCmpt = srcArchetype->At(cmptHash, srcIdx);
			auto dstCmpt = dstArchetype->At(cmptHash, dstIdx);
			size_t size = std::get<1>(srcCmpt);
			assert(size == std::get<1>(dstCmpt));
			memcpy(std::get<0>(dstCmpt), std::get<0>(srcCmpt), size);
		}

		size_t srcMovedIdx = srcArchetype->Erase(srcIdx);
		if (srcMovedIdx != static_cast<size_t>(-1)) {
			auto srcMovedEntityTarget = d2p.find({ srcArchetype, srcMovedIdx });
			auto srcMovedEntity = srcMovedEntityTarget->second;
			d2p.erase(srcMovedEntityTarget);
			d2p[{srcArchetype, srcIdx}] = srcMovedEntity;
			srcMovedEntity->idx() = srcMovedIdx;
		}

		d2p[{dstArchetype, dstIdx}] = e;

		e->archetype() = dstArchetype;
		e->idx() = dstIdx;

		if (srcArchetype->Size() == 0) {
			ids.erase(srcArchetype->id);
			id2a.erase(srcArchetype->id);
			delete srcArchetype;
		}
	}
}
