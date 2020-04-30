#pragma once

#include "Archetype.h"
#include "EntityBase.h"

#include "Job.h"

#include <UBL/Pool.h>

#include <mutex>

namespace Ubpa {
	class World;

	class ArchetypeMngr {
	public:
		ArchetypeMngr(World* w) : w{ w } {}

		~ArchetypeMngr();

		World* World() const noexcept { return w; }

		inline Archetype* GetArchetypeOf(const CmptIDSet& archetypeID);

		template<typename... Cmpts>
		inline Archetype* GetOrCreateArchetypeOf();
		
		template<typename AllList, typename AnyList, typename NoneList, typename LocateList>
		const std::set<Archetype*>& QueryArchetypes();

		template<typename... Cmpts>
		const std::tuple<EntityBase*, Cmpts*...> CreateEntity();

		// TODO: CreateEntities

		template<typename... Cmpts>
		const std::tuple<Cmpts*...> EntityAttach(EntityBase* e);

		template<typename... Cmpts>
		void EntityDetach(EntityBase* e);

		void Release(EntityBase* e);

		template<typename Sys>
		void GenJob(Job* job, Sys&& sys);

		void AddCommand(const std::function<void()>& command);
		void RunCommands();

	private:
		template<typename... Cmpts>
		static std::vector<size_t> TypeListToIDVec(TypeList<Cmpts...>);

		Ubpa::World* w;

		Pool<EntityBase> entityPool;

		std::map<std::tuple<Archetype*, size_t>, EntityBase*> ai2e; // (archetype, idx) -> entity

		std::set<CmptIDSet> ids;
		std::map<CmptIDSet, Archetype*> id2a; // id to archetype

		// Query Cache
		// TypeID<AllList, AnyList, NoneList, LocateList> to archetype set
		// AllList, AnyList, NoneList, LocateList are **sorted**
		std::unordered_map<size_t, std::set<Archetype*>> queryCache;
		struct Query {
			Query(const std::vector<size_t>& allCmptIDs,
				const std::vector<size_t>& anyCmptIDs,
				const std::vector<size_t>& noneCmptIDs,
				const std::vector<size_t>& locateCmptIDs)
				: allCmptIDs{ allCmptIDs },
				anyCmptIDs{ anyCmptIDs },
				noneCmptIDs{ noneCmptIDs },
				locateCmptIDs { locateCmptIDs }
			{
			}

			std::vector<size_t> allCmptIDs; // sorted
			std::vector<size_t> anyCmptIDs; // sorted
			std::vector<size_t> noneCmptIDs; // sorted
			std::vector<size_t> locateCmptIDs; // sorted
		};
		std::unordered_map<size_t, Query> id2query;

		// command
		std::vector<std::function<void()>> commandBuffer;
		std::mutex commandBufferMutex;
	};
}

#include "ArchetypeMngr.inl"
