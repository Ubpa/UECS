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
	// - Singleton: IsSingleton, GetSingletonEntity, GetSingleton
	// - other: EntityNum, AddCommand
	// [important]
	// - some API with CmptType need RTDCmptTraits to get {size|alignment|lifecycle function} (throw std::logic_error)
	// - API with Entity require Entity exist  (throw std::invalid_argument)
	class EntityMngr {
	public:
		RTDCmptTraits cmptTraits;

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

		// assert(Have(e, CmptType::Of<Cmpt>))
		template<typename Cmpt, typename... Args>
		Cmpt* Emplace(Entity, Args&&...);

		void Detach(Entity, const CmptType* types, size_t num);

		bool Have(Entity, CmptType) const;

		// nullptr if not containts <Cmpt>
		template<typename Cmpt>
		Cmpt* Get(Entity) const;
		// nullptr if not containts CmptType
		CmptPtr Get(Entity, CmptType) const;

		std::vector<CmptPtr> Components(Entity) const;

		bool Exist(Entity) const;

		void Destroy(Entity);

		size_t EntityNum(const EntityQuery&) const;

		std::tuple<bool, std::vector<CmptAccessPtr>> LocateSingletons(const SingletonLocator&) const;

		bool IsSingleton(CmptType) const;
		Entity GetSingletonEntity(CmptType) const;
		// nullptr if not singleton
		CmptPtr GetSingleton(CmptType) const;
		template<typename Cmpt>
		Cmpt* GetSingleton() const { return GetSingleton(CmptType::Of<Cmpt>).As<Cmpt>(); }

		// filter's all contains cmpt
		template<typename Cmpt>
		std::vector<Cmpt*> GetCmptArray(const ArchetypeFilter&) const;
		std::vector<CmptPtr> GetCmptArray(const ArchetypeFilter&, CmptType) const;
		std::vector<Entity> GetEntityArray(const ArchetypeFilter&) const;

		void Accept(IListener* listener) const;

	private:
		Pool<Chunk> sharedChunkPool; // destruct finally

		friend class World;
		friend class Archetype;

		EntityMngr() = default;

		static bool IsSet(const CmptType* types, size_t num) noexcept;

		template<typename... Cmpts>
		Archetype* GetOrCreateArchetypeOf();
		// types not contain Entity
		Archetype* GetOrCreateArchetypeOf(const CmptType* types, size_t num);

		// return original archetype
		template<typename... Cmpts>
		Archetype* AttachWithoutInit(Entity);
		Archetype* AttachWithoutInit(Entity, const CmptType* types, size_t num);

		const std::set<Archetype*>& QueryArchetypes(const EntityQuery&) const;
		mutable std::unordered_map<EntityQuery, std::set<Archetype*>> queryCache;

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
		void RecycleEntityEntry(Entity);

		std::unordered_map<CmptTypeSet, std::unique_ptr<Archetype>> ts2a; // archetype's CmptTypeSet to archetype
	};
}

#include "detail/EntityMngr.inl"
