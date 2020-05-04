#pragma once

#include "Entity.h"
#include "ScheduleRegistrar.h"
#include "detail/SystemMngr.h"

namespace Ubpa {
	class World : public SystemMngr {
	public:
		World();

		template<typename... Cmpts>
		std::tuple<Entity*, Cmpts*...> CreateEntity();

		// static OnStartSchedule
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
		// run commands later
		template<typename Sys>
		void Each(Sys&& s);

		// if Sys's return type is bool, Each stop when return false
		// **not** run commands
		template<typename Sys>
		void Each(Sys&& s) const;

		// run commands later
		template<typename Sys>
		void ParallelEach(Sys&& s);

		// **not** run commands
		template<typename Sys>
		void ParallelEach(Sys&& s) const;

		void AddCommand(const std::function<void()>& command);

	private:
		ArchetypeMngr mngr;

		ScheduleRegistrar<SysType::OnStart> startRegistrar;
		ScheduleRegistrar<SysType::OnUpdate> updateRegistrar;
		ScheduleRegistrar<SysType::OnStop> stopRegistrar;

		Job startJobGraph;
		Job updateJobGraph;
		Job stopJobGraph;

		mutable JobExecutor executor;
	};
}

#include "detail/World.inl"
