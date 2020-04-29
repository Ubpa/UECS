#pragma once

#include "../ScheduleRegistrar.h"
#include "SystemMngr.h"

#include <functional>

namespace Ubpa {
	class ArchetypeMngr;

	class CmptSysMngr {
	public:
		static CmptSysMngr& Instance() {
			static CmptSysMngr instance;
			return instance;
		}

		template<typename Cmpt>
		void Register();

		void GenSchedule(ScheduleRegistrar<SysType::OnStart>& registrar, const SystemMngr& sysMngr);
		void GenSchedule(ScheduleRegistrar<SysType::OnUpdate>& registrar, const SystemMngr& sysMngr);
		void GenSchedule(ScheduleRegistrar<SysType::OnStop>& registrar, const SystemMngr& sysMngr);

	private:
		std::vector<std::function<ScheduleFunc<SysType::OnStart>>> staticStartScheduleFuncs;
		std::vector<std::function<ScheduleFunc<SysType::OnUpdate>>> staticUpdateScheduleFuncs;
		std::vector<std::function<ScheduleFunc<SysType::OnStop>>> staticStopScheduleFuncs;

		std::vector<ScheduleFunc<SysType::OnStart>*> dynamicStartScheduleFuncs;
		std::vector<ScheduleFunc<SysType::OnUpdate>*> dynamicUpdateScheduleFuncs;
		std::vector<ScheduleFunc<SysType::OnStop>*> dynamicStopScheduleFuncs;

	private:
		CmptSysMngr() = default;
	};
}

#include "CmptSysMngr.inl"
