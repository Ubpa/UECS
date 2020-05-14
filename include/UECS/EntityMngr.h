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
		const std::tuple<Entity, Cmpts*...> CreateEntity();

		Entity Instantiate(Entity e);

		// TODO: CreateEntities

		template<typename... Cmpts>
		const std::tuple<Cmpts*...> Attach(Entity e);

		template<typename Cmpt, typename... Args>
		Cmpt* Emplace(Entity e, Args&&... args);

		template<typename... Cmpts>
		void Detach(Entity e);

		void Destroy(Entity e);

		bool Exist(Entity e) const;

		template<typename Cmpt>
		bool Have(Entity e) const;
		template<typename Cmpt>
		Cmpt* Get(Entity e) const;

		size_t EntityNum(const EntityQuery& query) const;

		void AddCommand(const std::function<void()>& command);

	private:
		friend class World;
		EntityMngr() = default;
		~EntityMngr();

		const std::set<Archetype*>& QueryArchetypes(const EntityQuery& query) const;

		template<typename... Cmpts>
		Archetype* GetOrCreateArchetypeOf();

		template<typename... Cmpts>
		std::tuple<std::array<bool, sizeof...(Cmpts)>, std::tuple<Cmpts*...>> AttachWithoutInit(Entity e);

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
