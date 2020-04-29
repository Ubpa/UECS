#pragma once

namespace Ubpa {
	template<typename System>
	void SystemMngr::Register() {
		static_assert(HaveAnySchedule<System>,
			"<System> has no any schdedule fuction");

		if constexpr (Require<HaveOnStartSchedule, System>)
			n2start.emplace(TypeID<System>, GetSchedule<System, SysType::OnStart>());
		if constexpr (Require<HaveOnUpdateSchedule, System>)
			n2update.emplace(TypeID<System>, GetSchedule<System, SysType::OnUpdate>());
		if constexpr (Require<HaveOnStopSchedule, System>)
			n2stop.emplace(TypeID<System>, GetSchedule<System, SysType::OnStop>());
	}

	template<typename System>
	bool SystemMngr::IsRegistered() const {
		static_assert(HaveAnySchedule<System>,
			"<System> has no any schdedule fuction");

		if constexpr (Require<HaveOnStartSchedule, System>)
			return n2start.find(TypeID<System>) != n2start.end();
		else if constexpr (Require<HaveOnUpdateSchedule, System>)
			return n2update.find(TypeID<System>) != n2update.end();
		else // if constexpr (Require<HaveOnStopSchedule, System>)
			return n2stop.find(TypeID<System>) != n2stop.end();
	}

	template<typename System>
	void SystemMngr::Deregister() noexcept {
		static_assert(HaveAnySchedule<System>,
			"<System> has no any schdedule fuction");

		if constexpr (Require<HaveOnStartSchedule, System>)
			n2start.erase(TypeID<System>);
		if constexpr (Require<HaveOnUpdateSchedule, System>)
			n2update.erase(TypeID<System>);
		if constexpr (Require<HaveOnStopSchedule, System>)
			n2stop.erase(TypeID<System>);
	}
}
