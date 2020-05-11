#pragma once

#include "Entity.h"
#include "SystemMngr.h"
#include "EntityMngr.h"

#include <mutex>

namespace Ubpa {
	class World {
	public:
		World() : schedule{ this } {}

		SystemMngr systemMngr;
		EntityMngr entityMngr;

		// static OnUpdateSchedule
		// parallel OnUpdate
		// Commands, one-by-one
		void Update();

		std::string DumpUpdateJobGraph();

		void AddCommand(const std::function<void()>& command);

	private:
		mutable JobExecutor executor;
		Schedule schedule;

		Job jobGraph;
		std::vector<Job*> jobs;
		Pool<Job> jobPool;

		// command
		std::vector<std::function<void()>> commandBuffer;
		std::mutex commandBufferMutex;
		void RunCommands();
	};
}
