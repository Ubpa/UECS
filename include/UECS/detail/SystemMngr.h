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

		void GenTaskflow(tf::Taskflow& taskflow);

	private:
		std::unordered_set<size_t> registedCmptID;
		SystemSchedule schedule;
		std::vector<std::function<void(SystemSchedule&)>> staticScheduleFuncs;
		std::vector<std::function<void(SystemSchedule&)>> dynamicScheduleFuncs;
	};
}

#include "SystemMngr.inl"
