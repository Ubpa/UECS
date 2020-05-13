#pragma once

#include "Entity.h"
#include "SystemMngr.h"
#include "EntityMngr.h"

namespace Ubpa {
	class World {
	public:
		World() : schedule{ &entityMngr, &systemMngr } {}

		SystemMngr systemMngr;
		EntityMngr entityMngr;

		// static OnUpdateSchedule
		// parallel OnUpdate
		// Commands, one-by-one
		void Update();

		std::string DumpUpdateJobGraph() const;

	private:
		mutable JobExecutor executor;
		Schedule schedule;

		Job jobGraph;
		std::vector<Job*> jobs;
		Pool<Job> jobPool;
	};
}
