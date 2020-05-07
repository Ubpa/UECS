#pragma once

#include "Archetype.h"
#include "EntityData.h"
#include "Job.h"

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
		
		template<typename AllList, typename AnyList, typename NoneList, typename LocateList>
		const std::set<Archetype*>& QueryArchetypes() const;

		template<typename... Cmpts>
		const std::tuple<EntityData*, Cmpts*...> CreateEntity();

		// TODO: CreateEntities

		template<typename... Cmpts>
		const std::tuple<Cmpts*...> EntityAttach(EntityData* e);

		template<typename Cmpt, typename... Args>
		Cmpt* EntityAssignAttach(EntityData* e, Args... args);

		template<typename... Cmpts>
		void EntityDetach(EntityData* e);

		void Release(EntityData* e);

		template<typename Sys>
		void GenJob(Job* job, Sys&& sys) const;

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

		// Query Cache
		// TypeID<AllList, AnyList, NoneList, LocateList> to archetype set
		// AllList, AnyList, NoneList, LocateList are **sorted**
		mutable std::unordered_map<size_t, std::set<Archetype*>> queryCache;
		struct Query {
			Query(const std::vector<CmptType>& allCmptTypes,
				const std::vector<CmptType>& anyCmptTypes,
				const std::vector<CmptType>& noneCmptTypes,
				const std::vector<CmptType>& locateCmptTypes)
				: allCmptTypes{ allCmptTypes },
				anyCmptTypes{ anyCmptTypes },
				noneCmptTypes{ noneCmptTypes },
				locateCmptTypes{ locateCmptTypes }
			{
			}

			std::vector<CmptType> allCmptTypes; // sorted
			std::vector<CmptType> anyCmptTypes; // sorted
			std::vector<CmptType> noneCmptTypes; // sorted
			std::vector<CmptType> locateCmptTypes; // sorted
		};
		mutable std::unordered_map<size_t, Query> queryHashmap;

		// command
		std::vector<std::function<void()>> commandBuffer;
		std::mutex commandBufferMutex;
	};
}

#include "EntityMngr.inl"
