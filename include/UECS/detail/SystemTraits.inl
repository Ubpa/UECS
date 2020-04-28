#pragma once

#include <_deps/nameof.hpp>

namespace Ubpa {
	template<typename Cmpt, SysType type, typename>
	constexpr auto GetSys() noexcept {
		if constexpr (type == SysType::OnStart)
			return &Cmpt::OnStart;
		else if constexpr (type == SysType::OnUpdate)
			return &Cmpt::OnUpdate;
		else // if constexpr (type == SysType::Stop)
			return &Cmpt::OnStop;
	}

	template<typename Cmpt, SysType type, typename>
	constexpr std::string_view DefaultSysName() noexcept {
		return nameof::nameof_type<decltype(GetSys<Cmpt, type>())>();
	}

	template<typename Cmpt, SysType type, typename>
	constexpr ScheduleType<type> GetSchedule() noexcept{
		if constexpr (type == SysType::OnStart)
			return MemFuncOf<void(SystemSchedule<SysType::OnStart>&)>::run(&Cmpt::OnSchedule);
		else if constexpr (type == SysType::OnUpdate)
			return MemFuncOf<void(SystemSchedule<SysType::OnUpdate>&)>::run(&Cmpt::OnSchedule);
		else // if constexpr (type == SysType::Stop)
			return MemFuncOf<void(SystemSchedule<SysType::OnStop>&)>::run(&Cmpt::OnSchedule);
	}
}
