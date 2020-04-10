#pragma once

#include "ArchetypeMngr.h"
#include "Pool.h"

#include <taskflow/taskflow.hpp>

namespace Ubpa::detail::SystemSchedule_ {
	template<typename ArgList>
	struct Schedule;
}

namespace Ubpa {
	using System = tf::Taskflow;

	class SystemSchedule {
	public:
		SystemSchedule(ArchetypeMngr* mngr);
		~SystemSchedule();

		void Clear();

		template<typename Func>
		SystemSchedule& Regist(Func&& func);

		bool GenTaskflow(tf::Taskflow& taskflow) const;

	private:
		struct RWSystems {
			System* writer{ nullptr };
			std::vector<System*> readers;
		};

		System* RequestSystem();

		bool IsDAG() const noexcept;

		ArchetypeMngr* const mngr;
		std::vector<System*> requestedSysVec;
		std::unordered_map<size_t, RWSystems> id2rw;
		Pool<System> syspool;

		template<typename ArgList>
		friend struct detail::SystemSchedule_::Schedule;
	};
}

#include "SystemSchedule.inl"
