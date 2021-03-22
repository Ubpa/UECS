#pragma once

#include "Entity.hpp"
#include "CommandBuffer.hpp"
#include "SystemMngr.hpp"
#include "EntityMngr.hpp"
#include "Schedule.hpp"

#include <UGraphviz/UGraphviz.hpp>

#include <mutex>

namespace Ubpa::UECS {
	class IListener;

	// SystemMngr + EntityMngr
	class World {
		std::unique_ptr<std::pmr::synchronized_pool_resource> world_rsrc; // init before entityMngr

	public:
		World();
		World(const World&);
		World(World&&) noexcept;
		~World();

		SystemMngr systemMngr;
		EntityMngr entityMngr;

		// 1. update schedule
		// 2. run job graph for several layers
		// 3. update version
		// 4. clear frame resource
		void Update();

		void AddCommand(std::function<void()> command, int layer);
		void AddCommandBuffer(CommandBuffer cb, int layer);

		// after running Update()
		// you can use graphviz to vistualize the graph
		std::string DumpUpdateJobGraph() const;

		// after running Update()
		// use CmptTraits' registered component name
		UGraphviz::Graph GenUpdateFrameGraph(int layer = 0) const;

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
		CommandBuffer RunEntityJob(
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
		CommandBuffer RunChunkJob(
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

		// you just can use it in a job within a frame
		std::pmr::synchronized_pool_resource* GetFrameSyncResource() { return &frame_sync_rsrc; }
		template<typename T, typename... Args>
		T* SyncCreateFrameObject(Args&&... args);

		std::pmr::synchronized_pool_resource* GetSyncResource() { return world_rsrc.get(); }

		std::uint64_t Version() const noexcept { return version; }
	private:
		bool inRunningJobGraph{ false };

		mutable JobExecutor executor;
		Schedule schedule;
		std::uint64_t version{ 0 };

		Job jobGraph;
		std::vector<Job*> jobs;
		std::unique_ptr<std::pmr::monotonic_buffer_resource> jobRsrc;

		// command
		std::map<int, CommandBuffer> lcommandBuffer;
		std::mutex commandBufferMutex;
		void RunCommands(int layer);

		CommandBuffer Run(SystemFunc*);

		std::pmr::synchronized_pool_resource frame_sync_rsrc;
	};
}

#include "details/World.inl"
