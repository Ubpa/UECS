#pragma once

#include "../SystemTraits.h"

#include <UTemplate/Func.h>

namespace Ubpa::detail::ScheduleRegistrar_ {
	template<typename Func>
	struct GenSystem;

	template<SysType type, typename... Args>
	struct ScheduleAdd;
}

namespace Ubpa {
	template<SysType type>
	template<typename Func>
	ScheduleRegistrar<type>& ScheduleRegistrar<type>::Register(const std::string& name, Func&& func) {
		detail::ScheduleRegistrar_::ScheduleAdd<type, FuncTraits_ArgList<std::remove_reference_t<Func>>>::run(mngr, schedule, std::forward<Func>(func), name);
		return *this;
	}

	template<SysType type>
	template<typename Cmpt, typename Func>
	ScheduleRegistrar<type>& ScheduleRegistrar<type>::Register(const std::string& name, Func Cmpt::* func) {
		Register(name, detail::ScheduleRegistrar_::GenSystem<Func Cmpt::*>::run(func));
		return *this;
	}

	template<SysType type>
	template<typename Cmpt, typename Func>
	ScheduleRegistrar<type>& ScheduleRegistrar<type>::Register(Func Cmpt::* func) {
		Register(std::string(nameof::nameof_type<Func Cmpt::*>()), func);
		return *this;
	}

	template<SysType type>
	template<typename Cmpt>
	ScheduleRegistrar<type>& ScheduleRegistrar<type>::Register() {
		static_assert(HaveCmptSys<Cmpt, type>, "<Cmpt> have no corresponding System (OnStart/OnUpdate/OnStop)");
		Register(GetCmptSys<Cmpt, type>());
		return *this;
	}

	template<SysType type>
	ScheduleRegistrar<type>& ScheduleRegistrar<type>::Order(std::string_view first, const std::string& second) {
		auto target = schedule.sysOrderMap.find(first);
		if (target != schedule.sysOrderMap.end())
			target->second.insert(second);
		else
			schedule.sysOrderMap.emplace(std::string(first), std::set<std::string>{second});

		return *this;
	}

	template<SysType type>
	template<typename CmptFirst, typename CmptSecond>
	ScheduleRegistrar<type>& ScheduleRegistrar<type>::Order() {
		return Order(DefaultSysName<CmptFirst, type>(), std::string(DefaultSysName<CmptSecond, type>()));
	}

	template<SysType type>
	template<typename Cmpt>
	ScheduleRegistrar<type>& ScheduleRegistrar<type>::Before(std::string_view name) {
		return Order(name, std::string(DefaultSysName<Cmpt, type>()));
	}

	template<SysType type>
	template<typename Cmpt>
	ScheduleRegistrar<type>& ScheduleRegistrar<type>::After(const std::string& name) {
		return Order(DefaultSysName<Cmpt, type>(), name);
	}
}

namespace Ubpa::detail::ScheduleRegistrar_ {
	template<SysType type, typename... Args>
	struct ScheduleAdd<type, TypeList<Args...>> {
		template<typename Sys>
		static auto run(ArchetypeMngr* mngr, Schedule& schedule, Sys&& sys, const std::string& name) noexcept {
			auto job = schedule.RequestJob(name);
			mngr->GenJob(job, sys);
			if (!job->empty())
				(Register<Args>(schedule, job), ...);
		}

		template<typename TagedCmpt>
		static void Register(Schedule& schedule, Job* job) {
			if constexpr (CmptTag::IsLastFrame_v<TagedCmpt>)
				schedule.id2rw[Ubpa::TypeID<CmptTag::RemoveTag_t<TagedCmpt>>].pre_readers.push_back(job);
			else if constexpr (CmptTag::IsWrite_v<TagedCmpt>)
				schedule.id2rw[Ubpa::TypeID<CmptTag::RemoveTag_t<TagedCmpt>>].writers.insert(job);
			else if constexpr (CmptTag::IsNewest_v<TagedCmpt>)
				schedule.id2rw[Ubpa::TypeID<CmptTag::RemoveTag_t<TagedCmpt>>].post_readers.push_back(job);
			else if constexpr (CmptTag::IsBefore_v<TagedCmpt>)
				RegisterBefore(schedule, job, typename TagedCmpt::CmptList{});
			else // if constexpr (CmptTag::IsAfter_v<TagedCmpt>)
				RegisterAfter(schedule, job, typename TagedCmpt::CmptList{});
		}

		template<typename... Cmpts>
		static void RegisterBefore(Schedule& schedule, Job* job, TypeList<Cmpts...>) {
			(schedule.sysOrderMap[job->name()].insert(std::string(DefaultSysName<Cmpts, type>())), ...);
		}

		template<typename... Cmpts>
		static void RegisterAfter(Schedule& schedule, Job* job, TypeList<Cmpts...>) {
			(schedule.sysOrderMap[std::string(DefaultSysName<Cmpts, type>())].insert(job->name()), ...);
		}
	};

	template<typename Cmpt, typename ArgList>
	struct GenSystemHelper;

	template<typename Cmpt, typename... Cmpts>
	struct GenSystemHelper<Cmpt, TypeList<Cmpts...>> {
		template<typename Func>
		static auto run(Func func) noexcept {
			if constexpr (FuncTraits<std::decay_t<Func>>::is_const) {
				return [func](const Cmpt* cmpt, Cmpts... cmpts) {
					(cmpt->*func)(cmpts...);
				};
			}
			else {
				return [func](Cmpt* cmpt, Cmpts... cmpts) {
					(cmpt->*func)(cmpts...);
				};
			}
		}
	};

	template<typename Func>
	struct GenSystem {
		static auto run(Func func) noexcept {
			return GenSystemHelper<FuncTraits_Obj<std::decay_t<Func>>, FuncTraits_ArgList<std::decay_t<Func>>>::run(std::forward<Func>(func));
		}
	};
}
