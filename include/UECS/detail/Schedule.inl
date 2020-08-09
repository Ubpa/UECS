#pragma once

namespace Ubpa::UECS {
	template<typename Func>
	const SystemFunc* Schedule::RegisterEntityJob(
		Func&& func,
		std::string name,
		ArchetypeFilter filter,
		CmptLocator cmptLocator,
		SingletonLocator singletonLocator)
	{
		return Request(
			std::forward<Func>(func),
			std::move(name),
			std::move(filter),
			std::move(cmptLocator),
			std::move(singletonLocator)
		);
	}


	template<typename Func>
	const SystemFunc* Schedule::RegisterChunkJob(
		Func&& func,
		std::string name,
		ArchetypeFilter filter,
		SingletonLocator singletonLocator
	) {
		return Request(
			std::forward<Func>(func),
			std::move(name),
			std::move(filter),
			std::move(singletonLocator)
		);
	}

	template<typename Func>
	const SystemFunc* Schedule::RegisterJob(
		Func&& func,
		std::string name,
		SingletonLocator singletonLocator
	) {
		return Request(
			std::forward<Func>(func),
			std::move(name),
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
