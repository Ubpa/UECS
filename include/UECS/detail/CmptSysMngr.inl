#pragma once

#include "../SystemTraits.h"

namespace Ubpa{
	template<typename Cmpt>
	void CmptSysMngr::Register() {
		if constexpr (Require<HaveOnStart, Cmpt>) {
			staticStartScheduleFuncs.push_back([](SystemSchedule<SysType::OnStart>& schedule) {
				schedule.Register<Cmpt>();
			});
		}
		if constexpr (Require<HaveOnUpdate, Cmpt>) {
			staticUpdateScheduleFuncs.push_back([](SystemSchedule<SysType::OnUpdate>& schedule) {
				schedule.Register<Cmpt>();
			});
		}
		if constexpr (Require<HaveOnStop, Cmpt>) {
			staticStopScheduleFuncs.push_back([](SystemSchedule<SysType::OnStop>& schedule) {
				schedule.Register<Cmpt>();
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
