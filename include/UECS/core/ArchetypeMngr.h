#pragma once

#include "Archetype.h"
#include "Pool.h"
#include "EntityData.h"

#include <UTemplate/Typelist.h>

namespace Ubpa {
	class World;
	class ArchetypeMngr {
	public:
		ArchetypeMngr(World* w) : w(w) {}

		~ArchetypeMngr() {
			for (auto p : id2a)
				delete p.second;
		}

		inline World* World() const noexcept { return w; }

		inline Archetype* GetArchetypeOf(const Archetype::ID& archetypeID) {
			auto target = id2a.find(archetypeID);
			assert(target != id2a.end());
			return target->second;
		}

		template<typename... Cmpts>
		inline Archetype* GetOrCreateArchetypeOf() {
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
		const std::vector<Archetype*> LocateArchetypeWith() {
			std::vector<Archetype*> rst;
			for (auto& p : id2a) {
				if (p.second->IsContain<Cmpts...>())
					rst.push_back(p.second);
			}
			return rst;
		}

		template<typename... Cmpts>
		EntityData* CreateEntity() {
			Archetype* archetype = GetOrCreateArchetypeOf<Cmpts...>();
			size_t idx = archetype->CreateEntity<Cmpts...>();

			auto entity = entityPool.request();
			entity->archetype() = archetype;
			entity->idx() = idx;
			d2p[*entity] = entity;

			return entity;
		}

		template<typename Cmpt, typename... Args>
		Cmpt* EntityAdd(EntityData* e, Args&&... args) {
			Archetype* srcArchetype = e->archetype();
			size_t srcIdx = e->idx();

			auto& srcID = srcArchetype->GetID();
			auto dstID = srcID;
			dstID.Add<Cmpt>();
			
			Archetype* dstArchetype;
			auto target = id2a.find(dstID);
			if (target == id2a.end()) {
				dstArchetype = new Archetype(srcArchetype, IType<Cmpt>{});
				assert(dstID == dstArchetype->GetID());
				id2a[dstID] = dstArchetype;
				ids.insert(dstID);
			}
			else
				dstArchetype = target->second;
			
			size_t dstIdx = dstArchetype->CreateEntity();
			Cmpt* cmpt = dstArchetype->Init<Cmpt>(dstIdx, std::forward<Args>(args)...);
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

			return cmpt;
		}

		void Release(EntityData* e) {
			size_t movedEntityIdx = e->archetype()->Erase(e->idx());

			auto target = d2p.find({ e->archetype(), movedEntityIdx });
			EntityData* movedEntity = target->second;
			d2p.erase(target);

			movedEntity->idx() = e->idx();
			d2p[*e] = movedEntity;

			entityPool.recycle(e);
		}

	private:
		Pool<EntityData> entityPool;
		std::map<EntityData, EntityData*> d2p;
		std::set<Archetype::ID> ids;
		Ubpa::World* w;
		std::map<Archetype::ID, Archetype*> id2a; // id to archetype
	};
}