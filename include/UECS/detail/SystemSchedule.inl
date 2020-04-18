#pragma once

#include <UTemplate/Func.h>
#include <_deps/nameof.hpp>

namespace Ubpa::detail::SystemSchedule_ {
	template<typename Func>
	struct GenSystem;
}

namespace Ubpa {
	template<typename Cmpt, typename Func>
	SystemSchedule& SystemSchedule::Regist(Func Cmpt::* func) {
		Regist(detail::SystemSchedule_::GenSystem<Func Cmpt::*>::run(func), nameof::nameof_type<Func Cmpt::*>());
		return *this;
	}

	template<typename Func>
	SystemSchedule& SystemSchedule::Regist(Func&& func, std::string_view name) {
		detail::SystemSchedule_::Schedule<FuncTraits_ArgList<std::remove_reference_t<Func>>>::run(this, std::forward<Func>(func), name);
		return *this;
	}
}

namespace Ubpa::detail::SystemSchedule_ {
	template<typename... Cmpts>
	struct Schedule<TypeList<Cmpts...>> {
		template<typename Func>
		static auto run(SystemSchedule* sysSchedule, Func&& func, std::string_view name) noexcept {
			auto system = sysSchedule->RequestSystem(name);
			sysSchedule->mngr->GenTaskflow(system, func);
			(Regist<std::remove_pointer_t<Cmpts>>(sysSchedule->id2rw, system), ...);
		}

		template<typename Cmpt>
		static void Regist(std::unordered_map<size_t, SystemSchedule::RWSystems>& id2rw, tf::Taskflow* system) {
			if constexpr (std::is_const_v<Cmpt>) {
				id2rw[Ubpa::TypeID<std::remove_const_t<Cmpt>>].readers.push_back(system);
			}
			else {
				id2rw[Ubpa::TypeID<Cmpt>].writers.push_back(system);
			}
		}
	};
}

namespace Ubpa::detail::SystemSchedule_ {
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
