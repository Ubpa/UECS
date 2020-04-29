#pragma once

#include "Schedule.h"
#include "detail/ArchetypeMngr.h"
#include "SystemTraits.h"

namespace Ubpa {
	template<SysType type>
	class ScheduleRegistrar {
	public:
		template<typename Func>
		ScheduleRegistrar& Register(const std::string& name, Func&& func);

		template<typename Cmpt, typename Func>
		ScheduleRegistrar& Register(const std::string& name, Func Cmpt::* func);

		// use nameof::nameof_type<Func Cmpt::*>()
		template<typename Cmpt, typename Func>
		ScheduleRegistrar& Register(Func Cmpt::* func);

		ScheduleRegistrar& Order(std::string_view first, const std::string& second);
		template<typename CmptFirst, typename CmptSecond>
		ScheduleRegistrar& Order();
		template<typename Cmpt>
		ScheduleRegistrar& Before(std::string_view name);
		template<typename Cmpt>
		ScheduleRegistrar& After(const std::string& name);

		// TODO: regist not parallel

	private:
		friend class World;

		ScheduleRegistrar(ArchetypeMngr* mngr) noexcept : mngr{ mngr } {}
		ArchetypeMngr* mngr;
		Schedule schedule;

		friend class CmptSysMngr;

		template<typename Cmpt>
		ScheduleRegistrar& Register();
	};
}

#include "detail/ScheduleRegistrar.inl"
