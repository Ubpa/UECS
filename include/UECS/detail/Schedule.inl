#pragma once

namespace Ubpa::UECS {
	template<typename Func>
	Schedule& Schedule::Register(Func&& func, std::string name, EntityFilter filter) {
		Request(std::forward<Func>(func), std::move(name), std::move(filter));
		return *this;
	}

	template<typename Func>
	Schedule& Schedule::Register(Func&& func, std::string name, EntityLocator locator, EntityFilter filter) {
		Request(std::forward<Func>(func), std::move(name), std::move(locator), std::move(filter));
		return *this;
	}

	template<typename... Args>
	void Schedule::Request(Args&&... args) {
		SystemFunc* sysFunc = sysFuncPool.Request(std::forward<Args>(args)...);
		sysFuncs.emplace(sysFunc->HashCode(), sysFunc);
	}

	inline Schedule::Schedule(EntityMngr* entityMngr, SystemMngr* systemMngr)
		: entityMngr{ entityMngr }, systemMngr{ systemMngr }{}

	inline Schedule& Schedule::LockFilter(std::string_view sys) {
		sysLockFilter.insert(SystemFunc::HashCode(sys));
		return *this;
	}
}
