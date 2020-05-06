#pragma once

#include "ScheduleRegistrar.h"

namespace Ubpa{
	class EntityMngr;

	class SystemMngr {
	public:
		template<typename... Systems>
		void Register();
		template<typename System>
		bool IsRegistered() const;
		template<typename System>
		void Deregister() noexcept;

		std::string DumpStartTaskflow() const;
		std::string DumpUpdateTaskflow() const;
		std::string DumpStopTaskflow() const;

	protected:
		SystemMngr(EntityMngr* entityMngr);
		
		// static OnStartSchedule
		// parallel OnStart
		void Start();

		// static OnUpdateSchedule
		// parallel OnUpdate
		void Update();

		// static OnStopSchedule
		// parallel OnStop
		void Stop();

		ScheduleRegistrar<SysType::OnStart> startRegistrar;
		ScheduleRegistrar<SysType::OnUpdate> updateRegistrar;
		ScheduleRegistrar<SysType::OnStop> stopRegistrar;

		Job startJobGraph;
		Job updateJobGraph;
		Job stopJobGraph;

		mutable JobExecutor executor;

	private:
		template<typename System>
		void RegisterOne();

	private:
		friend class CmptSysMngr;
		std::unordered_map<size_t, ScheduleFunc<SysType::OnStart>*> n2start;
		std::unordered_map<size_t, ScheduleFunc<SysType::OnUpdate>*> n2update;
		std::unordered_map<size_t, ScheduleFunc<SysType::OnStop>*> n2stop;
	};
}

#include "detail/SystemMngr.inl"
