#pragma once

#include <UTemplate/Concept.h>

#include <string_view>

namespace Ubpa {
	// [ Concept Usage ]
	// Requare<Concept, Cmpt>

	// enum
	enum class SysType {
		OnStart,
		OnUpdate,
		OnStop
	};

	template<SysType>
	class SystemSchedule;

	template<typename Cmpt>
	Concept(HaveOnStart, &Cmpt::OnStart);

	template<typename Cmpt>
	Concept(HaveOnUpdate, &Cmpt::OnUpdate);

	template<typename Cmpt>
	Concept(HaveOnStop, &Cmpt::OnStop);

	template<typename Cmpt>
	Concept(HaveOnStartSchedule, MemFuncOf<void(SystemSchedule<SysType::OnStart>&)>::run(&Cmpt::OnSchedule));

	template<typename Cmpt>
	Concept(HaveOnUpdateSchedule, MemFuncOf<void(SystemSchedule<SysType::OnUpdate>&)>::run(&Cmpt::OnSchedule));

	template<typename Cmpt>
	Concept(HaveOnStopSchedule, MemFuncOf<void(SystemSchedule<SysType::OnStop>&)>::run(&Cmpt::OnSchedule));

	template<typename Cmpt, SysType type>
	constexpr bool HaveSys = type == SysType::OnStart && Require<HaveOnStart, Cmpt>
		|| type == SysType::OnUpdate && Require<HaveOnUpdate, Cmpt>
		|| type == SysType::OnStop && Require<HaveOnStop, Cmpt>;

	template<typename Cmpt, SysType type>
	constexpr bool HaveSchedule = type == SysType::OnStart && Require<HaveOnStartSchedule, Cmpt>
		|| type == SysType::OnUpdate && Require<HaveOnUpdateSchedule, Cmpt>
		|| type == SysType::OnStop && Require<HaveOnStopSchedule, Cmpt>;

	template<typename Cmpt, SysType type,
		typename = std::enable_if_t<HaveSys<Cmpt, type>>>
	constexpr auto GetSys() noexcept;

	template<SysType type>
	using ScheduleType = void(*)(SystemSchedule<type>&);
	template<typename Cmpt, SysType type,
		typename = std::enable_if_t<HaveSchedule<Cmpt, type>>>
	constexpr ScheduleType<type> GetSchedule() noexcept;

	template<typename Cmpt, SysType type,
		typename = std::enable_if_t<HaveSys<Cmpt, type>>>
	constexpr std::string_view DefaultSysName() noexcept;
}

#include "detail/SystemTraits.inl"
