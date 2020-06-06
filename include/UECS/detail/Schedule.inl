#pragma once

namespace Ubpa {
	template<typename Func>
	Schedule& Schedule::Register(Func&& func, std::string name, EntityFilter filter) {
		return Request(std::forward<Func>(func), std::move(name), std::move(filter));
	}

	template<typename Func>
	Schedule& Schedule::Register(Func&& func, std::string name, EntityLocator locator, EntityFilter filter) {
		return Request(std::forward<Func>(func), std::move(name), std::move(locator), std::move(filter));
	}

	template<typename... Args>
	Schedule& Schedule::Request(Args&&... args) {
		SystemFunc* sysFunc = sysFuncPool.Request(std::forward<Args>(args)...);
		sysFuncs.emplace(sysFunc->HashCode(), sysFunc);
		return *this;
	}

	inline Schedule::Schedule(EntityMngr* entityMngr, SystemMngr* systemMngr)
		: entityMngr{ entityMngr }, systemMngr{ systemMngr }{}

	inline Schedule& Schedule::LockFilter(std::string_view sys) {
		sysLockFilter.insert(SystemFunc::HashCode(sys));
		return *this;
	}
}
