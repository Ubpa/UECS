#pragma once

#include <functional>

namespace Ubpa {
	class SystemSchedule;
	class ArchetypeMngr;

	class SystemMngr {
	public:
		static SystemMngr& Instance() {
			static SystemMngr instance;
			return instance;
		}

		template<typename Cmpt>
		void Regist();

		void GenStartSchedule(SystemSchedule& schedule);
		void GenUpdateSchedule(SystemSchedule& schedule);
		void GenStopSchedule(SystemSchedule& schedule);

	private:
		std::vector<std::function<void(SystemSchedule&)>> staticStartScheduleFuncs;
		std::vector<std::function<void(SystemSchedule&)>> staticUpdateScheduleFuncs;
		std::vector<std::function<void(SystemSchedule&)>> staticStopScheduleFuncs;

		std::vector<std::function<void(SystemSchedule&)>> dynamicStartScheduleFuncs;
		std::vector<std::function<void(SystemSchedule&)>> dynamicUpdateScheduleFuncs;
		std::vector<std::function<void(SystemSchedule&)>> dynamicStopScheduleFuncs;

	private:
		SystemMngr() = default;
	};
}

#include "SystemMngr.inl"
