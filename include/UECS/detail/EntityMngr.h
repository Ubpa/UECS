#pragma once

#include "../SystemFunc.h"
#include "Archetype.h"
#include "EntityData.h"
#include "Job.h"
#include "../EntityQuery.h"

#include <UContainer/Pool.h>

#include <mutex>

namespace Ubpa {
	class World;

	class EntityMngr {
	public:
		EntityMngr(World* w) : w{ w } {}

		~EntityMngr();

		World* World() const noexcept { return w; }

		template<typename... Cmpts>
		inline Archetype* GetOrCreateArchetypeOf();
		
		const std::set<Archetype*>& QueryArchetypes(const EntityQuery& query) const;

		template<typename... Cmpts>
		const std::tuple<EntityData*, Cmpts*...> CreateEntity();

		// TODO: CreateEntities

		template<typename... Cmpts>
		const std::tuple<Cmpts*...> EntityAttach(EntityData* e);

		template<typename Cmpt, typename... Args>
		Cmpt* EntityAssignAttach(EntityData* e, Args&&... args);

		template<typename... Cmpts>
		void EntityDetach(EntityData* e);

		void Release(EntityData* e);

		/*template<typename Sys>
		void GenJob(Job* job, Sys&& sys) const;*/
		void GenJob(Job* job, SystemFunc* sys) const;

		void AddCommand(const std::function<void()>& command);
		void RunCommands();

	private:
		template<typename... Cmpts>
		const std::tuple<Cmpts*...> EntityAttachWithoutInit(EntityData* e);

		template<typename... Cmpts>
		static std::vector<CmptType> TypeListToTypeVec(TypeList<Cmpts...>);

		Ubpa::World* w;

		Pool<EntityData> entityPool;

		std::map<std::tuple<Archetype*, size_t>, EntityData*> ai2e; // (archetype, idx) -> entity

		std::unordered_map<size_t, Archetype*> h2a; // CmptTypeSet's hashcode to archetype
		
		mutable std::unordered_map<EntityQuery, std::set<Archetype*>> queryCache;

		// command
		std::vector<std::function<void()>> commandBuffer;
		std::mutex commandBufferMutex;
	};
}

#include "EntityMngr.inl"
