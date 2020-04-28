#pragma once

#include "detail/ArchetypeMngr.h"

#include <UDP/Basic/xSTL/xMap.h>

#include <UBL/Pool.h>

#include <taskflow/taskflow.hpp>

#include "SystemTraits.h"

namespace Ubpa::detail::SystemSchedule_ {
	template<SysType type, typename ArgList>
	struct Schedule;
}

namespace Ubpa {
	using System = tf::Taskflow;

	template<SysType type>
	class SystemSchedule {
	public:
		template<typename Func>
		SystemSchedule& Regist(Func&& func, std::string_view name);

		// use nameof::nameof_type<Func Cmpt::*>()
		template<typename Cmpt, typename Func>
		SystemSchedule& Regist(Func Cmpt::* func);

		template<typename Cmpt,
			typename = std::enable_if_t<HaveSys<Cmpt,type>>>
		SystemSchedule& Regist();

		// TODO: not parallel
		/*template<typename Func>
		SystemSchedule& RegistNotParallel(Func&& func, std::string_view name);

		template<typename Cmpt, typename Func>
		SystemSchedule& RegistNotParallel(Func Cmpt::* func);*/

	private:
		friend class World;

		SystemSchedule(ArchetypeMngr* mngr);
		~SystemSchedule();

		void Clear();

		bool GenTaskflow(tf::Taskflow& taskflow) const;

		struct RWSystems {
			std::vector<System*> pre_readers;
			std::set<System*> writers;
			std::vector<System*> post_readers;
		};

		System* RequestSystem(std::string_view name);

		bool IsDAG() const noexcept;

		ArchetypeMngr* const mngr;
		std::unordered_map<size_t, RWSystems> id2rw;
		Pool<System> syspool;

		xMap<std::string, System*> systems;
		xMap<std::string, std::set<std::string>> sysOrderMap; // to children

		template<SysType type, typename ArgList>
		friend struct detail::SystemSchedule_::Schedule;
	};
}

#include "detail/SystemSchedule.inl"
