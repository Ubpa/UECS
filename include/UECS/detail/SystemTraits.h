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

	template<SysType type>
	class ScheduleRegistrar;

	template<SysType type>
	using ScheduleFunc = void(ScheduleRegistrar<type>&);

	template<typename Cmpt>
	Concept(HaveOnStart, &Cmpt::OnStart);

	template<typename Cmpt>
	Concept(HaveOnUpdate, &Cmpt::OnUpdate);

	template<typename Cmpt>
	Concept(HaveOnStop, &Cmpt::OnStop);

	template<typename System>
	Concept(HaveOnStartSchedule, MemFuncOf<ScheduleFunc<SysType::OnStart>>::run(&System::OnSchedule));

	template<typename System>
	Concept(HaveOnUpdateSchedule, MemFuncOf<ScheduleFunc<SysType::OnUpdate>>::run(&System::OnSchedule));

	template<typename System>
	Concept(HaveOnStopSchedule, MemFuncOf<ScheduleFunc<SysType::OnStop>>::run(&System::OnSchedule));

	template<typename Cmpt, SysType type>
	constexpr bool HaveCmptSys = ((type == SysType::OnStart) && Require<HaveOnStart, Cmpt>)
		|| ((type == SysType::OnUpdate) && Require<HaveOnUpdate, Cmpt>)
		|| ((type == SysType::OnStop) && Require<HaveOnStop, Cmpt>);

	template<typename System, SysType type>
	constexpr bool HaveSchedule = ((type == SysType::OnStart) && Require<HaveOnStartSchedule, System>)
		|| ((type == SysType::OnUpdate) && Require<HaveOnUpdateSchedule, System>)
		|| ((type == SysType::OnStop) && Require<HaveOnStopSchedule, System>);

	template<typename System>
	constexpr bool HaveAnySchedule = Require<HaveOnStartSchedule, System>
		|| Require<HaveOnUpdateSchedule, System>
		|| Require<HaveOnStopSchedule, System>;

	template<typename Cmpt, SysType type>
	constexpr auto GetCmptSys() noexcept;

	template<typename System, SysType type>
	constexpr ScheduleFunc<type>* GetSchedule() noexcept;

	template<typename Cmpt, SysType type>
	constexpr std::string_view DefaultSysName() noexcept;
}

#include "SystemTraits.inl"
