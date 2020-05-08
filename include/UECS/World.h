#pragma once

#include "Entity.h"
#include "SystemMngr.h"

namespace Ubpa {
	class World {
	public:
		SystemMngr systemMngr;

		World();

		template<typename... Cmpts>
		std::tuple<EntityPtr, Cmpts*...> CreateEntity();

		// static OnUpdateSchedule
		// parallel OnUpdate
		// Commands, one-by-one
		void Update();

		std::string DumpUpdateJobGraph();

		//// if Sys's return type is bool, Each stop when return false
		//// run commands later
		//template<typename Sys>
		//void Each(Sys&& s);

		//// if Sys's return type is bool, Each stop when return false
		//// **not** run commands
		//template<typename Sys>
		//void Each(Sys&& s) const;

		//// run commands later
		//template<typename Sys>
		//void ParallelEach(Sys&& s);

		//// **not** run commands
		//template<typename Sys>
		//void ParallelEach(Sys&& s) const;

		void AddCommand(const std::function<void()>& command);

	private:
		mutable JobExecutor executor;
		Schedule schedule;

		EntityMngr entityMngr;

		Job jobGraph;
		std::vector<Job*> jobs;
		Pool<Job> jobPool;
	};
}

#include "detail/World.inl"
