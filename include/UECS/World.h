#pragma once

#include "Entity.h"
#include "SystemMngr.h"
#include "EntityMngr.h"
#include "Schedule.h"

#include <UGraphviz/UGraphviz.h>

#include <mutex>

namespace Ubpa::UECS {
	class IListener;

	// SystemMngr + EntityMngr
	class World {
	public:
		World() :
			jobRsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>() },
			systemMngr { this } {}
		// not copy/move schedule, so you can't use DumpUpdateJobGraph() and GenUpdateFrameGraph() before Update()
		World(const World&);
		World(World&&) noexcept;
		~World();

		SystemMngr systemMngr;
		EntityMngr entityMngr;

		// 1. schedule: run registered System's static OnUpdate(Schedule&)
		// 2. gen job graph: schedule -> graph
		// 3. run job graph in worker threads
		// 4. run commands in main thread
		void Update();

		void AddCommand(std::function<void()> command, int layer = 0);

		// after running Update()
		// you can use graphviz to vistualize the graph
		std::string DumpUpdateJobGraph() const;

		// after running Update()
		// use RTDCmptTraits' registered component name
		UGraphviz::Graph GenUpdateFrameGraph() const;

		void Accept(IListener*) const;

		// you can't run several parallel jobs in parallel because there is only an executor

		// Func's argument list:
		// [const] World*
		// {LastFrame|Latest}<Singleton<Cmpt>>
		// SingletonsView
		// Entity
		// std::size_t indexInQuery
		// <tagged-components>: [const] <Cmpt>*...
		// CmptsView
		template<typename Func>
		void RunEntityJob(
			Func&&,
			bool isParallel = true,
			ArchetypeFilter = {},
			CmptLocator = {},
			SingletonLocator = {}
		);

		// Func's argument list:
		// const World*
		// {LastFrame|Latest}<Singleton<Cmpt>>
		// SingletonsView
		// Entity
		// std::size_t indexInQuery
		// <tagged-components>: const <Cmpt>*...
		// CmptsView
		// --
		// CmptLocator's Cmpt AccessMode can't be WRITE
		template<typename Func>
		void RunEntityJob(
			Func&&,
			bool isParallel = true,
			ArchetypeFilter = {},
			CmptLocator = {},
			SingletonLocator = {}
		) const;

		// Func's argument list:
		// [const] World*
		// {LastFrame|Latest}<Singleton<Cmpt>>
		// SingletonsView
		// std::size_t entityBeginIndexInQuery
		// ChunkView (necessary)
		template<typename Func>
		void RunChunkJob(
			Func&&,
			ArchetypeFilter = {},
			bool isParallel = true,
			SingletonLocator = {}
		);

		// Func's argument list:
		// const World*
		// {LastFrame|Latest}<Singleton<Cmpt>>
		// SingletonsView
		// std::size_t entityBeginIndexInQuery
		// ChunkView (necessary)
		// --
		// ArchetypeFilter's Cmpt AccessMode can't be WRITE
		template<typename Func>
		void RunChunkJob(
			Func&&,
			ArchetypeFilter = {},
			bool isParallel = true,
			SingletonLocator = {}
		) const;

		std::pmr::synchronized_pool_resource* GetFrameSyncResource() { return &frame_sync_rsrc; }
	private:
		bool inRunningJobGraph{ false };

		mutable JobExecutor executor;
		Schedule schedule;

		Job jobGraph;
		std::vector<Job*> jobs;
		std::unique_ptr<std::pmr::unsynchronized_pool_resource> jobRsrc;

		// command
		std::map<int, std::vector<std::function<void()>>> commandBuffer;
		std::mutex commandBufferMutex;
		void RunCommands();

		void Run(SystemFunc*);

		std::pmr::synchronized_pool_resource frame_sync_rsrc;
	};
}

#include "details/World.inl"
