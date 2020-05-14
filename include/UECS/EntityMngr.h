#pragma once

#include "SystemFunc.h"
#include "detail/Archetype.h"
#include "detail/Job.h"
#include "EntityQuery.h"

#include <UContainer/Pool.h>

#include <mutex>

namespace Ubpa {
	class World;

	class EntityMngr {
	public:
		template<typename... Cmpts>
		std::tuple<Entity, Cmpts*...> CreateEntity();
		Entity CreateEntity(CmptTypeSet types);

		Entity Instantiate(Entity e);

		// TODO: CreateEntities

		template<typename... Cmpts>
		std::tuple<Cmpts*...> Attach(Entity e);
		template<typename... CmptTypes>
		std::array<CmptPtr, sizeof...(CmptTypes)> Attach(Entity e, CmptTypes...);

		template<typename Cmpt, typename... Args>
		Cmpt* Emplace(Entity e, Args&&... args);

		template<typename... Cmpts>
		void Detach(Entity e);

		template<typename Cmpt>
		bool Have(Entity e) const;

		template<typename Cmpt>
		Cmpt* Get(Entity e) const;

		std::vector<CmptPtr> Components(Entity e) const;

		bool Exist(Entity e) const;

		void Destroy(Entity e);

		size_t EntityNum(const EntityQuery& query) const;

		void AddCommand(const std::function<void()>& command);

	private:
		friend class World;
		EntityMngr() = default;
		~EntityMngr();

		const std::set<Archetype*>& QueryArchetypes(const EntityQuery& query) const;

		template<typename... Cmpts>
		Archetype* GetOrCreateArchetypeOf();
		Archetype* GetOrCreateArchetypeOf(CmptTypeSet types);
		
		template<typename... CmptTypes> // <CmptTypes> == CmptType
		void AttachWithoutInit(Entity e, CmptTypes... types);

		void GenJob(Job* job, SystemFunc* sys) const;

		struct EntityInfo {
			Archetype* archetype{ nullptr };
			size_t idxInArchetype{ size_t_invalid };
			size_t version{ 0 }; // version
		};
		std::vector<EntityInfo> entityTable;
		std::vector<size_t> entityTableFreeEntry;
		size_t RequestEntityFreeEntry();
		void RecycleEntityEntry(Entity e);

		std::unordered_map<size_t, Archetype*> h2a; // archetype's hashcode to archetype
		
		mutable std::unordered_map<EntityQuery, std::set<Archetype*>> queryCache;

		// command
		std::vector<std::function<void()>> commandBuffer;
		std::mutex commandBufferMutex;
		void RunCommands();
	};
}

#include "detail/EntityMngr.inl"
