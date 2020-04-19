#pragma once

#include <UTemplate/Typelist.h>
#include <UTemplate/Func.h>
#include <UTemplate/Concept.h>

namespace Ubpa::detail::SystemMngr_ {
	template<typename T>
	Concept(HaveOnRegist, &T::OnRegist);

	template<typename T>
	Concept(HaveOnStart, &T::OnStart);

	template<typename T>
	Concept(HaveOnUpdate, &T::OnUpdate);

	template<typename T>
	Concept(HaveOnStop, &T::OnStop);

	template<typename T>
	Concept(HaveOnStartSchedule, &T::OnStartSchedule);

	template<typename T>
	Concept(HaveOnUpdateSchedule, &T::OnUpdateSchedule);

	template<typename T>
	Concept(HaveOnStopSchedule, &T::OnStopSchedule);
}

namespace Ubpa{
	template<typename Cmpt>
	void SystemMngr::Regist() {
		if (registedCmptID.find(TypeID<Cmpt>) != registedCmptID.end())
			return;
		registedCmptID.insert(TypeID<Cmpt>);

		if constexpr (Require<detail::SystemMngr_::HaveOnRegist, Cmpt>) {
			static bool flag = false;
			if (!flag) {
				Cmpt::OnRegist();
				flag = false;
			}
		}

		if constexpr (Require<detail::SystemMngr_::HaveOnStart, Cmpt>) {
			staticStartScheduleFuncs.push_back([](SystemSchedule& schedule) {
				schedule.Regist(&Cmpt::OnStart);
			});
		}
		if constexpr (Require<detail::SystemMngr_::HaveOnUpdate, Cmpt>) {
			staticUpdateScheduleFuncs.push_back([](SystemSchedule& schedule) {
				schedule.Regist(&Cmpt::OnUpdate);
			});
		}
		if constexpr (Require<detail::SystemMngr_::HaveOnStop, Cmpt>) {
			staticStopScheduleFuncs.push_back([](SystemSchedule& schedule) {
				schedule.Regist(&Cmpt::OnStop);
			});
		}

		if constexpr (Require<detail::SystemMngr_::HaveOnStartSchedule, Cmpt>)
			dynamicStartScheduleFuncs.push_back(&Cmpt::OnStartSchedule);
		if constexpr (Require<detail::SystemMngr_::HaveOnUpdateSchedule, Cmpt>)
			dynamicUpdateScheduleFuncs.push_back(&Cmpt::OnUpdateSchedule);
		if constexpr (Require<detail::SystemMngr_::HaveOnStopSchedule, Cmpt>)
			dynamicStopScheduleFuncs.push_back(&Cmpt::OnStopSchedule);
	}
}
