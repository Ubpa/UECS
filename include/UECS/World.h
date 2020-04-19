#pragma once

#include "Entity.h"
#include "detail/SystemMngr.h"

#include <UTemplate/Func.h>

namespace Ubpa::detail::World_ {
	template<typename Args>
	struct Each;
	template<typename Args>
	struct ParallelEach;
}

namespace Ubpa {
	class World {
	public:
		World();

		template<typename... Cmpts>
		std::tuple<Entity*, Cmpts*...> CreateEntity();

		// static OnUpdateSchedule
		// parallel OnStart
		// Commands, one-by-one
		void Start();

		// static OnUpdateSchedule
		// parallel OnUpdate
		// Commands, one-by-one
		void Update();

		// static OnStopSchedule
		// parallel OnStop
		// Commands, one-by-one
		void Stop();

		std::string DumpStartTaskflow() const;
		std::string DumpUpdateTaskflow() const;
		std::string DumpStopTaskflow() const;

		// if Sys's return type is bool, Each stop when return false
		template<typename Sys>
		void Each(Sys&& s);

		// if Sys's return type is bool, Each stop when return false
		template<typename Sys>
		void Each(Sys&& s) const;

		template<typename Sys>
		void ParallelEach(Sys&& s);

		template<typename Sys>
		void ParallelEach(Sys&& s) const;

	private:
		SystemMngr sysMngr;
		tf::Taskflow startTaskflow;
		tf::Taskflow updateTaskflow;
		tf::Taskflow stopTaskflow;
		tf::Executor executor;
		ArchetypeMngr mngr;

		template<typename ArgList>
		friend struct detail::World_::Each;
		template<typename ArgList>
		friend struct detail::World_::ParallelEach;
	};
}

#include "detail/World.inl"
