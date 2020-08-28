#pragma once

namespace Ubpa::UECS {
	template<typename Func>
	void World::RunEntityJob(
		Func&& func,
		bool isParallel,
		ArchetypeFilter filter,
		CmptLocator cmptLocator,
		SingletonLocator singletonLocator
	) {
		SystemFunc sys{
			std::forward<Func>(func),
			"",
			std::move(filter),
			std::move(cmptLocator),
			std::move(singletonLocator),
			std::move(isParallel)
		};
		Run(&sys);
	}

	template<typename Func>
	void World::RunChunkJob(
		Func&& func,
		ArchetypeFilter filter,
		bool isParallel,
		SingletonLocator singletonLocator
	) {
		SystemFunc sys{
			std::forward<Func>(func),
			"",
			std::move(filter),
			std::move(singletonLocator),
			std::move(isParallel)
		};
		Run(&sys);
	}

	template<typename Func>
	void World::RunJob(
		Func&& func,
		SingletonLocator singletonLocator
	) {
		SystemFunc sys{
			std::forward<Func>(func),
			"",
			std::move(singletonLocator)
		};
		Run(&sys);
	}
}
