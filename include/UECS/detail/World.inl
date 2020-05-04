#pragma once

#include "../CmptRegistrar.h"

#include <UTemplate/Func.h>

namespace Ubpa::detail::World_ {
	template<typename ArgList, typename CmptList, typename OtherArgList>
	struct Each;
	template<typename CmptList>
	struct ParallelEach;
}

namespace Ubpa {
	template<typename Sys>
	void World::Each(Sys&& s) {
		using ArgList = FuncTraits_ArgList<Sys>;
		using TagedCmptList = CmptTag::GetTimePointList_t<ArgList>;
		using OtherArgList = CmptTag::RemoveTimePoint_t<ArgList>;
		detail::World_::Each<ArgList, TagedCmptList, OtherArgList>::run(mngr, std::forward<Sys>(s));
	}

	template<typename Sys>
	void World::Each(Sys&& s) const {
		using ArgList = FuncTraits_ArgList<Sys>;
		using TagedCmptList = CmptTag::GetTimePointList_t<ArgList>;
		using OtherArgList = CmptTag::RemoveTimePoint_t<ArgList>;
		detail::World_::Each<ArgList, TagedCmptList, OtherArgList>::run(mngr, std::forward<Sys>(s));
	}

	template<typename Sys>
	void World::ParallelEach(Sys&& s) {
		using ArgList = FuncTraits_ArgList<Sys>;
		using TagedCmptList = CmptTag::GetTimePointList_t<ArgList>;
		detail::World_::ParallelEach<TagedCmptList>::run(mngr, executor, std::forward<Sys>(s));
	}

	template<typename Sys>
	void World::ParallelEach(Sys&& s) const {
		using ArgList = FuncTraits_ArgList<Sys>;
		using TagedCmptList = CmptTag::GetTimePointList_t<ArgList>;
		detail::World_::ParallelEach<TagedCmptList>::run(mngr, executor, std::forward<Sys>(s));
	}

	template<typename... Cmpts>
	std::tuple<Entity*, Cmpts*...> World::CreateEntity() {
		static_assert(IsSet_v<TypeList<Cmpts...>>, "Componnents must be different");
		(CmptLifecycleMngr::Instance().Register<Cmpts>(),...);
		auto rst = mngr.CreateEntity<Cmpts...>();
		assert("[ERROR] hasn't registed <Cmpts>"
			&& CmptRegistrar::Instance().template IsRegistered<Cmpts...>());
		return {reinterpret_cast<Entity*>(std::get<0>(rst)),
			std::get<1 + Find_v<TypeList<Cmpts...>, Cmpts>>(rst)...};
	}
}

namespace Ubpa::detail::World_ {
	template<typename... Args, typename... Cmpts, typename... OtherArgs>
	struct Each<TypeList<Args...>, TypeList<Cmpts * ...>, TypeList<OtherArgs...>> {
		static_assert(sizeof...(Cmpts) > 0);
		using CmptList = TypeList<std::remove_const_t<Cmpts>...>;
		using AllList = CmptTag::ConcatedAllList_t<TypeList<Args...>>;
		using AnyList = CmptTag::ConcatedAnyList_t<TypeList<Args...>>;
		using NoneList = CmptTag::ConcatedNoneList_t<TypeList<Args...>>;
		static_assert(IsSet_v<CmptList>, "Componnents must be different");

		template<typename Sys>
		static void run_common(const ArchetypeMngr& mngr, Sys&& s) {
			for (const Archetype* archetype : mngr.QueryArchetypes<AllList, AnyList, NoneList, CmptList>()) {
				auto cmptsTupleVec = archetype->Locate<std::remove_const_t<Cmpts>...>();
				size_t num = archetype->Size();
				size_t chunkNum = archetype->ChunkNum();
				size_t chunkCapacity = archetype->ChunkCapacity();

				for (size_t i = 0; i < chunkNum; i++) {
					size_t J = std::min(chunkCapacity, num - (i * chunkCapacity));
					for (size_t j = 0; j < J; j++) {
						auto unsortedArgTuple = std::make_tuple(static_cast<Cmpts*>(std::get<Find_v<CmptList, std::remove_const_t<Cmpts>>>(cmptsTupleVec[i]) + j)..., OtherArgs{}...);
						if constexpr (std::is_same_v<FuncTraits_Ret<Sys>, bool>) {
							if (!s(std::get<Args>(unsortedArgTuple)...))
								return;
						}
						else
							s(std::get<Args>(unsortedArgTuple)...);
					}
				}
			}
		}

		template<typename Sys>
		static void run(ArchetypeMngr& mngr, Sys&& s) {
			run_common(mngr, std::forward<Sys>(s));
			mngr.RunCommands();
		}

		template<typename Sys>
		static void run(const ArchetypeMngr& mngr, Sys&& s) {
			static_assert((std::is_const_v<Cmpts> &&...),
				"arguments must be const <Component>*");

			run_common(mngr, std::forward<Sys>(s));
		}
	};

	template<typename... Cmpts>
	struct ParallelEach<TypeList<Cmpts* ...>> {
		static_assert(sizeof...(Cmpts) > 0);
		using CmptList = TypeList<std::remove_const_t<Cmpts>...>;
		static_assert(IsSet_v<CmptList>, "Componnents must be different");

		template<typename Sys>
		static void run_common(const ArchetypeMngr& mngr, JobExecutor& executor, Sys&& s) {
			Job job;
			mngr.GenJob(&job, std::forward<Sys>(s));
			if (job.empty())
				return;

			executor.run(job).wait();
		}

		template<typename Sys>
		static void run(ArchetypeMngr& mngr, JobExecutor& executor, Sys&& s) {
			run_common(mngr, executor, std::forward<Sys>(s));
			mngr.RunCommands();
		}

		template<typename Sys>
		static void run(const ArchetypeMngr& mngr, JobExecutor& executor, Sys&& s) {
			static_assert((std::is_const_v<Cmpts> &&...),
				"arguments must be const <Component>*");
			run_common(mngr, executor, std::forward<Sys>(s));
		}
	};
}
