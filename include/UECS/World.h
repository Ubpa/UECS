#pragma once

#include "Entity.h"
#include "SystemMngr.h"
#include "EntityMngr.h"

#include <UGraphviz/UGraphviz.h>

#include <mutex>

namespace Ubpa::UECS {
	class IListener;

	// SystemMngr + EntityMngr
	class World {
	public:
		World();

		SystemMngr systemMngr;
		EntityMngr entityMngr;

		// 1. schedule: run registered System's static OnUpdate(Schedule&)
		// 2. gen job graph: schedule -> graph
		// 3. run job graph in worker threads
		// 4. run commands in main thread
		void Update();

		// after running Update
		// you can use graphviz to vistualize the graph
		std::string DumpUpdateJobGraph() const;

		// after running Update
		// use RTDCmptTraits' registered component name
		UGraphviz::Graph GenUpdateFrameGraph() const;

		void Accept(IListener*) const;

		void AddCommand(std::function<void(World*)> command);

		// Func's argument list:
		// World*
		// {LastFrame|Latest}<Singleton<Cmpt>>
		// SingletonsView
		// Entity
		// size_t indexInQuery
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
		// World*
		// {LastFrame|Latest}<Singleton<Cmpt>>
		// SingletonsView
		// ChunkView (necessary)
		template<typename Func>
		void RunChunkJob(
			Func&&,
			ArchetypeFilter = {},
			bool isParallel = true,
			SingletonLocator = {}
		);

		// Func's argument list:
		// World*
		// {LastFrame|Write|Latest}<Singleton<Cmpt>>
		// SingletonsView
		template<typename Func>
		void RunJob(
			Func&&,
			SingletonLocator = {}
		);

	private:
		mutable JobExecutor executor;
		Schedule schedule;

		Job jobGraph;
		std::vector<Job*> jobs;
		Pool<Job> jobPool;

		// command
		std::vector<std::function<void(World*)>> commandBuffer;
		std::mutex commandBufferMutex;
		void RunCommands();

		void Run(SystemFunc*);

		// ==================================================
		World(const World& world) = delete;
		World(World&& world) = delete;
		World& operator==(World&& world) = delete;
		World& operator=(const World& world) = delete;
	};
}

#include "detail/World.inl"
