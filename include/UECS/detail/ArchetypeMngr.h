#pragma once

#include "Archetype.h"
#include "EntityBase.h"

#include <UBL/Pool.h>

#include <taskflow/taskflow.hpp>

#include <mutex>

namespace Ubpa {
	class World;

	class ArchetypeMngr {
	public:
		ArchetypeMngr(World* w) : w{ w } {}

		~ArchetypeMngr();

		inline World* World() const noexcept { return w; }

		inline Archetype* GetArchetypeOf(const CmptIDSet& archetypeID);

		template<typename... Cmpts>
		inline Archetype* GetOrCreateArchetypeOf();

		// TODO: Query Cache
		// query is static
		template<typename... Cmpts>
		const std::set<Archetype*>& GetArchetypeWith();

		template<typename... Cmpts>
		const std::tuple<EntityBase*, Cmpts*...> CreateEntity();

		// TODO: CreateEntities

		template<typename... Cmpts>
		const std::tuple<Cmpts*...> EntityAttach(EntityBase* e);

		template<typename... Cmpts>
		void EntityDetach(EntityBase* e);

		void Release(EntityBase* e);

		template<typename Sys>
		void GenTaskflow(tf::Taskflow* taskflow, Sys&& sys);

		void AddCommand(const std::function<void()>& command);
		void RunCommands();

	private:
		Pool<EntityBase> entityPool;

		std::map<std::tuple<Archetype*, size_t>, EntityBase*> ai2e; // (archetype, idx) -> entity

		std::set<CmptIDSet> ids;
		std::map<CmptIDSet, Archetype*> id2a; // id to archetype
		// TypeID<Typelist<Cmpts...>> to archetype set
		// Typelist<Cmpts...> is sorted
		std::unordered_map<size_t, std::set<Archetype*>> cmpts2as;
		// TypeID<Typelist<Cmpts...>> to Cmpt ID set
		// Typelist<Cmpts...> is sorted
		std::unordered_map<size_t, CmptIDSet> cmpts2ids;

		Ubpa::World* w;
		std::vector<std::function<void()>> commandBuffer;
		std::mutex commandBufferMutex;
	};
}

#include "ArchetypeMngr.inl"
