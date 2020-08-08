#pragma once

namespace Ubpa::UECS {
	template<typename Func>
	const SystemFunc* Schedule::Register(
		Func&& func,
		std::string name,
		ArchetypeFilter filter,
		SingletonLocator singletonLocator)
	{
		return Request(
			std::forward<Func>(func),
			std::move(name),
			std::move(filter),
			std::move(singletonLocator)
		);
	}

	template<typename Func>
	const SystemFunc* Schedule::Register(
		Func&& func,
		std::string name,
		CmptLocator locator,
		ArchetypeFilter filter,
		SingletonLocator singletonLocator
	) {
		return Request(
			std::forward<Func>(func),
			std::move(name),
			std::move(locator),
			std::move(filter),
			std::move(singletonLocator)
		);
	}

	template<typename... Args>
	const SystemFunc* Schedule::Request(Args&&... args) {
		SystemFunc* sysFunc = sysFuncPool.Request(std::forward<Args>(args)...);
		sysFuncs.emplace(sysFunc->HashCode(), sysFunc);
		return sysFunc;
	}

	inline Schedule& Schedule::LockFilter(std::string_view sys) {
		sysLockFilter.insert(SystemFunc::HashCode(sys));
		return *this;
	}
}
