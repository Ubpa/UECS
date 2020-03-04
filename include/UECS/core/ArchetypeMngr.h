#pragma once

#include "Archetype.h"
#include "pool.h"
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
		inline Archetype* GetOrCreateArchetypeOf();

		template<typename... Cmpts>
		const std::vector<Archetype*> GetArchetypeWith();

		template<typename... Cmpts>
		const std::tuple<EntityData*, Cmpts*...> CreateEntity();

		template<typename... Cmpts>
		const std::tuple<Cmpts*...> EntityAttach(EntityData* e);

		template<typename... Cmpts>
		void EntityDetach(EntityData* e);

		void Release(EntityData* e);

	private:
		pool<EntityData> entityPool;
		std::map<EntityData, EntityData*> d2p;
		std::set<Archetype::ID> ids;
		Ubpa::World* w;
		std::map<Archetype::ID, Archetype*> id2a; // id to archetype
	};
}

#include "detail/ArchetypeMngr.inl"
