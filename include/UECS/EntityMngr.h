#pragma once

#include "RTDCmptTraits.h"
#include "EntityQuery.h"
#include "SingletonLocator.h"
#include "CmptPtr.h"

#include "details/Job.h"
#include "details/TypeIDSet.h"

#include <memory_resource>
#include <small_vector.h>

namespace Ubpa::UECS {
	class World;
	class SystemFunc;
	class IListener;
	class Archetype;

	// Entity Manager of World
	// auto maintain Component's lifecycle ({default|copy|move} constructor, destructor)
	// [API]
	// - Entity: Create, Instantiate, Destroy, Exist
	// - Component: Attach, Emplace, Detach, Have, Get, Components
	// - Singleton: IsSingleton, GetSingletonEntity, GetSingleton
	// - other: EntityNum, AddCommand
	// [important]
	// - some API with TypeID need RTDCmptTraits to get {size|alignment|lifecycle function} (throw std::logic_error)
	// - API with Entity require Entity exist  (throw std::invalid_argument)
	// [details]
	// - when free entries is empty, use new entity entry (version is 0)
	class EntityMngr {
	public:
		EntityMngr();
		EntityMngr(const EntityMngr& em);
		EntityMngr(EntityMngr&&) noexcept;
		~EntityMngr();

		RTDCmptTraits cmptTraits;

		Entity Create(std::span<const TypeID> types = {});
		Entity Create(TypeID type) { return Create({ &type, 1 }); }

		Entity Instantiate(Entity);

		// TODO: CreateEntities

		void Attach(Entity, std::span<const TypeID> types);
		void Detach(Entity, std::span<const TypeID> types);
		bool Have(Entity, TypeID) const;

		// nullptr if not containts TypeID
		CmptPtr Get(Entity, TypeID) const;
		template<typename Cmpt>
		Cmpt* Get(Entity e) const { return Get(e, TypeID_of<Cmpt>).template As<Cmpt>(); }

		std::vector<CmptPtr> Components(Entity) const;

		bool Exist(Entity) const noexcept;

		void Destroy(Entity);

		std::size_t TotalEntityNum() const noexcept { return entityTable.size() - entityTableFreeEntry.size(); }
		std::size_t EntityNum(const EntityQuery&) const;
		// use entry in reverse
		std::span<const std::size_t> GetEntityFreeEntries() const noexcept { return { entityTableFreeEntry.data(), entityTableFreeEntry.size() }; }
		std::size_t GetEntityVersion(std::size_t idx) const noexcept { return entityTable[idx].version; }

		bool IsSingleton(TypeID) const;
		Entity GetSingletonEntity(TypeID) const;
		// nullptr if not singleton
		CmptPtr GetSingleton(TypeID) const;
		template<typename Cmpt>
		Cmpt* GetSingleton() const { return GetSingleton(TypeID_of<Cmpt>).template As<Cmpt>(); }

		std::vector<CmptPtr> GetCmptArray(const ArchetypeFilter&, TypeID) const;
		std::vector<Entity> GetEntityArray(const ArchetypeFilter&) const;

		void Accept(IListener* listener) const;

		EntityMngr& operator=(EntityMngr&&) noexcept = delete;
		EntityMngr& operator=(const EntityMngr&) = delete;

		void Clear();
	private:
		friend class World;
		friend class Archetype;

		static bool IsSet(std::span<const TypeID> types) noexcept;

		// types not contain Entity
		Archetype* GetOrCreateArchetypeOf(std::span<const TypeID> types);

		small_vector<CmptAccessPtr, 16> LocateSingletons(const SingletonLocator&) const;

		const std::set<Archetype*>& QueryArchetypes(const EntityQuery&) const;
		mutable std::unordered_map<EntityQuery, std::set<Archetype*>> queryCache;

		// if job is nullptr, direct run
		void GenEntityJob(World*, Job*, SystemFunc*) const;
		// if job is nullptr, direct run
		void GenChunkJob(World*, Job*, SystemFunc*) const;
		// if job is nullptr, direct run
		void GenJob(World*, Job*, SystemFunc*) const;
		// if job is nullptr, direct run
		void AutoGen(World*, Job*, SystemFunc*) const;

		struct EntityInfo {
			Archetype* archetype{ nullptr };
			std::size_t idxInArchetype{ static_cast<std::size_t>(-1) };
			std::size_t version{ 0 }; // version
		};
		std::vector<EntityInfo> entityTable;
		std::vector<std::size_t> entityTableFreeEntry;
		std::size_t RequestEntityFreeEntry();
		void RecycleEntityEntry(Entity);

		std::unique_ptr<std::pmr::unsynchronized_pool_resource> rsrc;
		std::unordered_map<TypeIDSet, std::unique_ptr<Archetype>> ts2a; // archetype's TypeIDSet to archetype
	};
}
