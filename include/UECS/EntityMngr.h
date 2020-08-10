#pragma once

#include "SystemFunc.h"
#include "detail/Archetype.h"
#include "detail/Job.h"
#include "EntityQuery.h"

#include <UContainer/Pool.h>

#include <memory>

namespace Ubpa::UECS {
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

		Entity Instantiate(Entity);

		// TODO: CreateEntities

		template<typename... Cmpts>
		std::tuple<Cmpts*...> Attach(Entity);

		// use RTDCmptTraits
		void Attach(Entity, const CmptType* types, size_t num);

		template<typename Cmpt, typename... Args>
		Cmpt* Emplace(Entity, Args&&...);

		// use RTDCmptTraits
		void Detach(Entity, const CmptType* types, size_t num);

		bool Have(Entity, CmptType) const;

		template<typename Cmpt>
		Cmpt* Get(Entity) const;
		CmptPtr Get(Entity, CmptType) const;

		std::vector<CmptPtr> Components(Entity) const;

		bool Exist(Entity) const;

		void Destroy(Entity);

		size_t EntityNum(const EntityQuery&) const;

		std::tuple<bool, std::vector<CmptPtr>> LocateSingletons(const SingletonLocator&) const;

		bool IsSingleton(CmptType) const;
		Entity GetSingletonEntity(CmptType) const;
		CmptPtr GetSingleton(CmptType) const;
		// nullptr if not singleton
		CmptPtr GetIfSingleton(CmptType) const;
		template<typename Cmpt>
		Cmpt* GetSingleton() const { return GetSingleton(CmptType::Of<Cmpt>).As<Cmpt>(); }
		template<typename Cmpt>
		Cmpt* GetIfSingleton() const { return GetIfSingleton(CmptType::Of<Cmpt>).As<Cmpt>(); }

		void Accept(IListener* listener) const;

	private:
		friend class World;
		EntityMngr() = default;

		static bool IsSet(const CmptType* types, size_t num);

		const std::set<Archetype*>& QueryArchetypes(const EntityQuery& query) const;

		template<typename... Cmpts>
		Archetype* GetOrCreateArchetypeOf();
		Archetype* GetOrCreateArchetypeOf(const CmptType* types, size_t num);

		template<typename... Cmpts>
		void AttachWithoutInit(Entity);
		void AttachWithoutInit(Entity, const CmptType* types, size_t num);

		void GenEntityJob(World*, Job*, SystemFunc*) const;
		void GenChunkJob(World*, Job*, SystemFunc*) const;
		void GenJob(World*, Job*, SystemFunc*) const;

		struct EntityInfo {
			Archetype* archetype{ nullptr };
			size_t idxInArchetype{ size_t_invalid };
			size_t version{ 0 }; // version
		};
		std::vector<EntityInfo> entityTable;
		std::vector<size_t> entityTableFreeEntry;
		size_t RequestEntityFreeEntry();
		void RecycleEntityEntry(Entity e);

		std::unordered_map<CmptTypeSet, std::unique_ptr<Archetype>> ts2a; // archetype's CmptTypeSet to archetype
		
		mutable std::unordered_map<EntityQuery, std::set<Archetype*>> queryCache;
	};
}

#include "detail/EntityMngr.inl"
