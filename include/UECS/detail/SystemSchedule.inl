#pragma once

#include <UTemplate/Func.h>

namespace Ubpa::detail::SystemSchedule_ {
	template<typename Func>
	struct GenSystem;
}

namespace Ubpa {
	template<typename Func>
	SystemSchedule& SystemSchedule::Regist(Func&& func) {
		if constexpr (std::is_member_function_pointer_v<Func>) {
			Regist(detail::SystemSchedule_::GenSystem<Func>::run(std::forward<Func>(func)));
		}
		else {
			detail::SystemSchedule_::Schedule<FuncTraits_ArgList<std::remove_reference_t<Func>>>::run(this, std::forward<Func>(func));
		}
		return *this;
	}
}

namespace Ubpa::detail::SystemSchedule_ {
	template<typename... Cmpts>
	struct Schedule<TypeList<Cmpts...>> {
		template<typename Func>
		static auto run(SystemSchedule* sysSchedule, Func&& func) noexcept {
			auto system = sysSchedule->RequestSystem();
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
		static auto run(Func&& func) noexcept {
			if constexpr (FuncTraits<std::decay_t<Func>>::is_const) {
				return [func = std::forward<Func>(func)](const Cmpt* cmpt, Cmpts... cmpts) {
					(cmpt->*func)(cmpts...);
				};
			}
			else {
				return [func = std::forward<Func>(func)](Cmpt* cmpt, Cmpts... cmpts) {
					(cmpt->*func)(cmpts...);
				};
			}
		}
	};

	template<typename Func>
	struct GenSystem {
		static auto run(Func&& func) noexcept {
			return GenSystemHelper<FuncTraits_Obj<std::decay_t<Func>>, FuncTraits_ArgList<std::decay_t<Func>>>::run(std::forward<Func>(func));
		}
	};
}
