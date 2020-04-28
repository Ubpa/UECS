#pragma once

#include "detail/ArchetypeMngr.h"

#include <UBL/Pool.h>

#include <taskflow/taskflow.hpp>

#include "SystemTraits.h"

#include <UDP/Basic/xSTL/xMap.h>
#include <UDP/Basic/Read.h>

namespace Ubpa::detail::SystemSchedule_ {
	template<SysType type, typename ArgList>
	struct Schedule;
}

namespace Ubpa {
	using System = tf::Taskflow;

	template<SysType type>
	class SystemSchedule {
	public:
		class Config {
		public:
			Read<Config, std::vector<std::string>> befores;
			Read<Config, std::vector<std::string>> afters;

			Config& Before(const std::string& name);

			// use nameof::nameof_type<Func Cmpt::*>()
			template<typename Cmpt, typename Func>
			Config& Before(Func Cmpt::* func);

			template<typename Cmpt>
			Config& Before();

			Config& After(const std::string& name);

			// use nameof::nameof_type<Func Cmpt::*>()
			template<typename Cmpt, typename Func>
			Config& After(Func Cmpt::* func);

			template<typename Cmpt>
			Config& After();
		};


		template<typename Func>
		SystemSchedule& Regist(Func&& func, std::string_view name, const Config& config = Config{});

		template<typename Cmpt, typename Func>
		SystemSchedule& Regist(Func Cmpt::* func, std::string_view name, const Config& config = Config{});

		// use nameof::nameof_type<Func Cmpt::*>()
		template<typename Cmpt, typename Func>
		SystemSchedule& Regist(Func Cmpt::* func, const Config& config = Config{});

		// TODO: regist not parallel

	private:
		friend class World;
		friend class SystemMngr;

		SystemSchedule(ArchetypeMngr* mngr);
		~SystemSchedule();

		template<typename Cmpt>
		SystemSchedule& Regist();

		void Clear();

		bool GenTaskflow(tf::Taskflow& taskflow) const;

		struct RWSystems {
			std::vector<System*> pre_readers;
			std::set<System*> writers;
			std::vector<System*> post_readers;
		};

		System* RequestSystem(const std::string& name);

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
