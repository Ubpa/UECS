#pragma once

#include "../SystemSchedule.h"
#include <UDP/Basic/xSTL/xMap.h>

#include <functional>

namespace Ubpa {
	class SystemMngr {
	public:
		SystemMngr(ArchetypeMngr* archetypeMngr);

		template<typename Cmpt>
		void Regist();

		void GenStartTaskflow(tf::Taskflow& taskflow);
		void GenUpdateTaskflow(tf::Taskflow& taskflow);
		void GenStopTaskflow(tf::Taskflow& taskflow);

	private:
		std::unordered_set<size_t> registedCmptID;

		SystemSchedule startSchedule;
		SystemSchedule updateSchedule;
		SystemSchedule stopSchedule;

		std::vector<std::function<void(SystemSchedule&)>> staticStartScheduleFuncs;
		std::vector<std::function<void(SystemSchedule&)>> staticUpdateScheduleFuncs;
		std::vector<std::function<void(SystemSchedule&)>> staticStopScheduleFuncs;

		std::vector<std::function<void(SystemSchedule&)>> dynamicStartScheduleFuncs;
		std::vector<std::function<void(SystemSchedule&)>> dynamicUpdateScheduleFuncs;
		std::vector<std::function<void(SystemSchedule&)>> dynamicStopScheduleFuncs;
	};
}

#include "SystemMngr.inl"
