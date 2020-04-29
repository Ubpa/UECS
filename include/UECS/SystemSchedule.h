#pragma once

#include "detail/ArchetypeMngr.h"

#include "SystemTraits.h"

#include <UBL/Pool.h>

#include <UDP/Basic/xSTL/xMap.h>
#include <UDP/Basic/Read.h>

#include <taskflow/taskflow.hpp>

namespace Ubpa::detail::SystemSchedule_ {
	template<SysType type, typename ArgList>
	struct Schedule;
}

namespace Ubpa {
	using Job = tf::Taskflow;

	template<SysType type>
	class SystemSchedule {
	public:
		template<typename Func>
		SystemSchedule& Register(const std::string& name, Func&& func);

		template<typename Cmpt, typename Func>
		SystemSchedule& Register(const std::string& name, Func Cmpt::* func);

		// use nameof::nameof_type<Func Cmpt::*>()
		template<typename Cmpt, typename Func>
		SystemSchedule& Register(Func Cmpt::* func);

		SystemSchedule& Order(std::string_view first, const std::string& second);
		template<typename CmptFirst, typename CmptSecond>
		SystemSchedule& Order();
		template<typename Cmpt>
		SystemSchedule& Before(std::string_view name);
		template<typename Cmpt>
		SystemSchedule& After(const std::string& name);

		// TODO: regist not parallel

	private:
		friend class World;
		friend class CmptSysMngr;

		SystemSchedule(ArchetypeMngr* mngr);
		~SystemSchedule();

		template<typename Cmpt>
		SystemSchedule& Register();

		void Clear();

		bool GenTaskflow(tf::Taskflow& taskflow) const;

		struct RW_Jobs {
			std::vector<Job*> pre_readers;
			std::set<Job*> writers;
			std::vector<Job*> post_readers;
		};

		Job* RequestJob(const std::string& name);

		bool IsDAG() const noexcept;

		ArchetypeMngr* const mngr;
		std::unordered_map<size_t, RW_Jobs> id2rw;
		Pool<Job> jobPool;

		xMap<std::string, Job*> jobs;
		xMap<std::string, std::set<std::string>> sysOrderMap; // to children

		template<SysType type, typename ArgList>
		friend struct detail::SystemSchedule_::Schedule;
	};
}

#include "detail/SystemSchedule.inl"
