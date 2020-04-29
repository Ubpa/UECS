#pragma once

#include "../SystemTraits.h"

namespace Ubpa{
	template<typename Cmpt>
	void CmptSysMngr::Register() {
		if constexpr (Require<HaveOnStart, Cmpt>) {
			staticStartScheduleFuncs.push_back([](ScheduleRegistrar<SysType::OnStart>& registrar) {
				registrar.Register<Cmpt>();
			});
		}
		if constexpr (Require<HaveOnUpdate, Cmpt>) {
			staticUpdateScheduleFuncs.push_back([](ScheduleRegistrar<SysType::OnUpdate>& registrar) {
				registrar.Register<Cmpt>();
			});
		}
		if constexpr (Require<HaveOnStop, Cmpt>) {
			staticStopScheduleFuncs.push_back([](ScheduleRegistrar<SysType::OnStop>& registrar) {
				registrar.Register<Cmpt>();
			});
		}

		if constexpr (Require<HaveOnStartSchedule, Cmpt>)
			dynamicStartScheduleFuncs.push_back(GetSchedule<Cmpt, SysType::OnStart>());
		if constexpr (Require<HaveOnUpdateSchedule, Cmpt>)
			dynamicUpdateScheduleFuncs.push_back(GetSchedule<Cmpt, SysType::OnUpdate>());
		if constexpr (Require<HaveOnStopSchedule, Cmpt>)
			dynamicStopScheduleFuncs.push_back(GetSchedule<Cmpt, SysType::OnStop>());
	}
}
