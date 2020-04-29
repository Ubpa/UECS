#pragma once

#include "../Schedule.h"

namespace Ubpa{
	class SystemMngr {
	public:
		template<typename System>
		void Register();
		template<typename System>
		bool IsRegistered() const;
		template<typename System>
		void Deregister() noexcept;

	private:
		friend class CmptSysMngr;
		std::unordered_map<size_t, ScheduleFunc<SysType::OnStart>*> n2start;
		std::unordered_map<size_t, ScheduleFunc<SysType::OnUpdate>*> n2update;
		std::unordered_map<size_t, ScheduleFunc<SysType::OnStop>*> n2stop;
	};
}

#include "SystemMngr.inl"
