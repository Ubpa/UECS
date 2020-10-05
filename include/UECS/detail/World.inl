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
	void World::RunEntityJob(
		Func&& func,
		bool isParallel,
		ArchetypeFilter filter,
		CmptLocator cmptLocator,
		SingletonLocator singletonLocator
	) const {
		using ArgList = FuncTraits_ArgList<Func>;
		static_assert(Contain_v<ArgList, World*> == 0,
			"const RunEntityJob should use const World*");
		static_assert(Length_v<Filter_t<ArgList, IsWrite>> == 0,
			"const RunEntityJob can't write cmpt");
		assert("const RunEntityJob can't write cmpt"
			&& !cmptLocator.HasWriteCmptType());

		const_cast<World*>(this)->RunEntityJob(
			std::forward<Func>(func),
			isParallel,
			std::move(filter),
			std::move(cmptLocator),
			std::move(singletonLocator)
		);
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
	void World::RunChunkJob(
		Func&& func,
		ArchetypeFilter filter,
		bool isParallel,
		SingletonLocator singletonLocator
	) const {
		using ArgList = FuncTraits_ArgList<Func>;
		static_assert(Contain_v<ArgList, World*> == 0,
			"const RunChunkJob should use const World*");
		assert("const RunChunkJob can't write cmpt"
			&& !filter.HaveWriteCmptType());

		const_cast<World*>(this)->RunChunkJob(
			std::forward<Func>(func),
			std::move(filter),
			isParallel,
			std::move(singletonLocator)
		);
	}
}
