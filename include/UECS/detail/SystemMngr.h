#pragma once

#include "ArchetypeMngr.h"

#include <taskflow/taskflow.hpp>

#include <UTemplate/Typelist.h>
#include <UTemplate/Func.h>
#include <UTemplate/Concept.h>

#include <unordered_map>

namespace Ubpa::detail::SystemMngr_ {
	template<typename T>
	Concept(HaveOnUpdate, &T::OnUpdate);

	template<typename Cmpt>
	struct Schedule;
}

namespace Ubpa {
	class SystemMngr {
	public:
		using System = tf::Taskflow;

		struct RWSystems {
			System* writer{ nullptr };
			std::vector<System*> readers;
		};

		struct Table {
			System finalSys;
			std::unordered_map<size_t, System> tasks;
			std::unordered_map<size_t, RWSystems> id2rw;
		};

		void GenTaskflow(Table& table) const;

		template<typename Cmpt>
		void Regist(ArchetypeMngr* mngr) {
			if constexpr (Require<detail::SystemMngr_::HaveOnUpdate, Cmpt>) {
				if (schedules.find(TypeID<Cmpt>) == schedules.end()) {
					schedules[TypeID<Cmpt>] = detail::SystemMngr_::Schedule<Cmpt>::run(mngr);
				}
			}
		}

	private:
		static bool IsDAG(const Table& table);

	private:
		std::unordered_map<size_t, std::function<void(Table& table)>> schedules;
	};
}

namespace Ubpa::detail::SystemMngr_ {
	template<typename Cmpt, typename ArgList>
	struct ScheduleHelper;

	template<typename Cmpt, typename... Cmpts>
	struct ScheduleHelper<Cmpt, TypeList<Cmpts...>> {

		static auto run(ArchetypeMngr* mngr) noexcept {
			return [mngr](SystemMngr::Table& table) {
				// schedule
				auto taskflow = &table.tasks[Ubpa::TypeID<Cmpt>];
				if constexpr (FuncTraits<decltype(&Cmpt::OnUpdate)>::is_const) {
					mngr->GenTaskflow(*taskflow, [](const Cmpt* cmpt, Cmpts... cmpts) {
						cmpt->OnUpdate(cmpts...);
					});
					Regist<const Cmpt>(table, taskflow);
				}
				else {
					mngr->GenTaskflow(*taskflow, [](Cmpt* cmpt, Cmpts... cmpts) {
						cmpt->OnUpdate(cmpts...);
					});
					Regist<Cmpt>(table, taskflow);
				}

				(Regist<std::remove_pointer_t<Cmpts>>(table, taskflow), ...);
			};
		}

		template<typename Cmpt>
		static void Regist(SystemMngr::Table& table, tf::Taskflow* taskflow) {
			if constexpr (std::is_const_v<Cmpt>) {
				using RawCmpt = std::remove_const_t<Cmpt>;
				table.id2rw[Ubpa::TypeID<RawCmpt>].readers.push_back(taskflow);
			}
			else {
				assert(table.id2rw[Ubpa::TypeID<Cmpt>].writer == nullptr
					&& "two component write same component");
				table.id2rw[Ubpa::TypeID<Cmpt>].writer = taskflow;
			}
		}
	};

	template<typename Cmpt>
	struct Schedule {
		static_assert(Require<HaveOnUpdate, Cmpt>);
		static auto run(ArchetypeMngr* mngr) noexcept {
			return ScheduleHelper<Cmpt, FuncTraits_ArgList<decltype(&Cmpt::OnUpdate)>>::run(mngr);
		}
	};
}
