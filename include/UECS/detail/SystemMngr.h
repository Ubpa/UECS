#pragma once

#include "SystemSchedule.h"

#include <functional>

namespace Ubpa {
	class SystemMngr {
	public:
		enum class ScheduleType {
			OnUpdate,
		};

		SystemMngr(ArchetypeMngr* archetypeMngr);
		~SystemMngr();

		template<typename Cmpt>
		void Regist();

		template<typename Func>
		void Regist(ScheduleType type, Func&& func);

		void GenTaskflow(std::map<ScheduleType, tf::Taskflow>& type2tf);

	private:
		std::unordered_set<size_t> registedCmptID;
		std::map<ScheduleType, SystemSchedule*> type2schedule;
		std::map<ScheduleType, std::vector<std::function<void(SystemSchedule*)>>> type2StaticScheduleFuncs;
		std::vector<std::function<void(std::map<ScheduleType, SystemSchedule*>&)>> dynamicScheduleFuncs;
	};
}

#include "SystemMngr.inl"
