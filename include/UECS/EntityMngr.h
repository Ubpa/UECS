#pragma once

#include "SystemFunc.h"
#include "detail/Archetype.h"
#include "detail/Job.h"
#include "EntityQuery.h"

#include <UContainer/Pool.h>

#include <mutex>

namespace Ubpa {
	class World;

	class IListener;

	// Entity Manager of World
	// auto maintain Component's lifecycle ({default|copy|move} constructor, destructor)
	// [API]
	// - Entity: Create, Instantiate, Destroy, Exist
	// - Component: Attach, Emplace, Detach, Have, Get, Components
	// - other: EntityNum, AddCommand
	// [important]
	// - API with CmptType need RTDCmptTraits to get {size|alignment|lifecycle function} (throw std::logic_error)
	// - API with Entity require Entity exist  (throw std::invalid_argument)
	class EntityMngr {
	public:
		template<typename... Cmpts>
		std::tuple<Entity, Cmpts*...> Create();
		// use RTDCmptTraits
		Entity Create(const CmptType* types, size_t num);
		// call Create(const CmptType*, size_t)
		template<typename... CmptTypes,
			// for function overload
			typename = std::enable_if_t<(std::is_same_v<CmptTypes, CmptType>&&...)>>
		Entity Create(CmptTypes...);

		Entity Instantiate(Entity);

		// TODO: CreateEntities

		template<typename... Cmpts>
		std::tuple<Cmpts*...> Attach(Entity);
		// use RTDCmptTraits
		void Attach(Entity, const CmptType* types, size_t num);
		template<typename... CmptTypes,
			// for function overload
			typename = std::enable_if_t<(std::is_same_v<CmptTypes, CmptType>&&...)>>
		std::array<CmptPtr, sizeof...(CmptTypes)> Attach(Entity, CmptTypes...);

		template<typename Cmpt, typename... Args>
		Cmpt* Emplace(Entity, Args&&...);

		template<typename... Cmpts>
		void Detach(Entity);
		// use RTDCmptTraits
		void Detach(Entity, const CmptType* types, size_t num);
		// call Detach(Entity, const CmptType*, size_t);
		template<typename... CmptTypes,
			// for function overload
			typename = std::enable_if_t<(std::is_same_v<CmptTypes, CmptType>&&...)>>
		void Detach(Entity, CmptTypes...);

		template<typename Cmpt>
		bool Have(Entity) const;
		bool Have(Entity, CmptType) const;

		template<typename Cmpt>
		Cmpt* Get(Entity) const;
		CmptPtr Get(Entity, CmptType) const;

		std::vector<CmptPtr> Components(Entity) const;

		bool Exist(Entity) const;

		void Destroy(Entity);

		size_t EntityNum(const EntityQuery&) const;

		void AddCommand(const std::function<void()>& command);

		void Accept(IListener* listener) const;

	private:
		friend class World;
		EntityMngr() = default;
		~EntityMngr();

		const std::set<Archetype*>& QueryArchetypes(const EntityQuery& query) const;

		template<typename... Cmpts>
		Archetype* GetOrCreateArchetypeOf();
		Archetype* GetOrCreateArchetypeOf(const CmptType* types, size_t num);
		// call GetOrCreateArchetypeOf(const CmptType*, size_t)
		template<typename... CmptTypes,
			// for function overload
			typename = std::enable_if_t<(std::is_same_v<CmptTypes, CmptType>&&...)>>
		Archetype* GetOrCreateArchetypeOf(CmptTypes...);

		template<typename... Cmpts>
		void AttachWithoutInit(Entity);
		void AttachWithoutInit(Entity, const CmptType* types, size_t num);
		// call AttachWithoutInit(Entity, const CmptType*, size_t)
		template<typename... CmptTypes,
			// for function overload
			typename = std::enable_if_t<(std::is_same_v<CmptTypes, CmptType>&&...)>>
		void AttachWithoutInit(Entity, CmptTypes...);

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
