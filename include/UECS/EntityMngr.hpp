#pragma once

#include "CmptTraits.hpp"
#include "EntityQuery.hpp"
#include "SingletonLocator.hpp"
#include "CmptPtr.hpp"

#include "details/Job.hpp"

#include <memory_resource>
#include <USmallFlat/small_vector.hpp>

namespace Ubpa::UECS {
	class World;
	class SystemFunc;
	class IListener;
	class Archetype;
	class synchronized_monotonic_buffer_resource;

	// Entity Manager of World
	// auto maintain Component's lifecycle ({default|copy|move} constructor, destructor)
	// [API]
	// - Entity: Create, Instantiate, Destroy, Exist
	// - Component: Attach, Emplace, Detach, Have, Get, Components
	// - Singleton: IsSingleton, GetSingletonEntity, GetSingleton
	// - other: EntityNum, AddCommand
	// [important]
	// - some API with TypeID need CmptTraits to get {size|alignment|lifecycle function} (throw std::logic_error)
	// - API with Entity require Entity exist  (throw std::invalid_argument)
	// [details]
	// - when free entries is empty, use new entity entry (version is 0)
	class EntityMngr {
	public:
		CmptTraits cmptTraits;

		Entity Create(std::span<const TypeID> types = {});

		Entity Instantiate(Entity);

		// TODO: CreateEntities

		void Attach(Entity, std::span<const TypeID> types);
		void Detach(Entity, std::span<const TypeID> types);
		bool Have(Entity, TypeID) const;

		// nullptr if not containts TypeID
		CmptAccessPtr GetComponent(Entity, AccessTypeID) const;
		CmptAccessPtr WriteComponent(Entity e, TypeID t) const { return GetComponent(e, { t, AccessMode::WRITE }); }
		CmptAccessPtr ReadComponent(Entity e, TypeID t) const { return GetComponent(e, { t, AccessMode::LATEST }); }
		template<typename Cmpt>
		Cmpt* WriteComponent(Entity e) const { return WriteComponent(e, TypeID_of<Cmpt>).template As<Cmpt, AccessMode::WRITE>(); }
		template<typename Cmpt>
		const Cmpt* ReadComponent(Entity e) const { return ReadComponent(e, TypeID_of<Cmpt>).template As<Cmpt, AccessMode::LATEST>(); }

		std::vector<CmptAccessPtr> Components(Entity, AccessMode) const;
		std::vector<CmptAccessPtr> WriteComponents(Entity e) const { return Components(e, AccessMode::WRITE); }
		std::vector<CmptAccessPtr> ReadComponents(Entity e) const { return Components(e, AccessMode::LATEST); }

		bool Exist(Entity) const noexcept;

		void Destroy(Entity);

		std::size_t TotalEntityNum() const noexcept { return entityTable.size() - entityTableFreeEntry.size(); }
		std::size_t EntityNum(const EntityQuery&) const;
		// use entry in reverse
		std::span<const std::size_t> GetEntityFreeEntries() const noexcept { return { entityTableFreeEntry.data(), entityTableFreeEntry.size() }; }
		std::size_t GetEntityVersion(std::size_t idx) const noexcept { return entityTable[idx].version; }

		bool IsSingleton(TypeID) const;
		Entity GetSingletonEntity(TypeID) const;
		CmptAccessPtr GetSingleton(AccessTypeID) const;
		CmptAccessPtr WriteSingleton(TypeID type) const { return GetSingleton({ type, AccessMode::WRITE }); }
		CmptAccessPtr ReadSingleton(TypeID type) const { return GetSingleton({ type, AccessMode::LATEST }); }
		template<typename Cmpt>
		Cmpt* WriteSingleton() const { return WriteSingleton(TypeID_of<Cmpt>).template As<Cmpt, AccessMode::WRITE>(); }
		template<typename Cmpt>
		const Cmpt* ReadSingleton() const { return ReadSingleton(TypeID_of<Cmpt>).template As<Cmpt, AccessMode::LATEST>(); }

		void Accept(IListener* listener) const;

		EntityMngr& operator=(EntityMngr&&) noexcept = delete;
		EntityMngr& operator=(const EntityMngr&) = delete;

		void Clear();

	private:
		friend class World;

		EntityMngr(World* world);
		EntityMngr(const EntityMngr&, World* world);
		EntityMngr(EntityMngr&&, World* world) noexcept;
		~EntityMngr();

		World* world;

		static bool IsSet(std::span<const TypeID> types) noexcept;

		// types not contain Entity
		Archetype* GetOrCreateArchetypeOf(std::span<const TypeID> types);

		small_vector<CmptAccessPtr> LocateSingletons(const SingletonLocator&) const;

		const std::set<Archetype*>& QueryArchetypes(const EntityQuery&) const;
		mutable std::unordered_map<EntityQuery, std::set<Archetype*>> queryCache;

		// if job is nullptr, direct run
		bool GenEntityJob(World*, Job*, SystemFunc*, int layer) const;
		// if job is nullptr, direct run
		bool GenChunkJob(World*, Job*, SystemFunc*, int layer) const;
		// if job is nullptr, direct run
		bool GenJob(World*, Job*, SystemFunc*) const;
		// if job is nullptr, direct run
		bool AutoGen(World*, Job*, SystemFunc*, int layer) const;

		struct EntityInfo {
			Archetype* archetype{ nullptr };
			std::size_t chunkIdx{ static_cast<std::size_t>(-1) };
			std::size_t idxInChunk{ static_cast<std::size_t>(-1) };
			std::uint64_t version{ 0 }; // version
		};
		std::vector<EntityInfo> entityTable;
		std::vector<std::size_t> entityTableFreeEntry;
		std::size_t RequestEntityFreeEntry();
		void RecycleEntityEntry(Entity);

		struct TypeIDSetHash {
			std::size_t operator()(const small_flat_set<TypeID>& types) const noexcept;
		};
		std::unordered_map<small_flat_set<TypeID>, std::unique_ptr<Archetype>, TypeIDSetHash> ts2a; // archetype's TypeIDSet to archetype
		void NewFrame() noexcept;
	};
}
