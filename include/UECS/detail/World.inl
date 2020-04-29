#pragma once

#include "../CmptRegistrar.h"

#include <UTemplate/Func.h>

namespace Ubpa {
	template<typename Sys>
	void World::Each(Sys&& s) {
		detail::World_::Each<typename FuncTraits<Sys>::ArgList>::run(this, std::forward<Sys>(s));
	}

	template<typename Sys>
	void World::Each(Sys&& s) const {
		detail::World_::Each<typename FuncTraits<Sys>::ArgList>::run(this, std::forward<Sys>(s));
	}

	template<typename Sys>
	void World::ParallelEach(Sys&& s) {
		detail::World_::ParallelEach<typename FuncTraits<Sys>::ArgList>::run(this, std::forward<Sys>(s));
	}

	template<typename Sys>
	void World::ParallelEach(Sys&& s) const {
		detail::World_::ParallelEach<typename FuncTraits<Sys>::ArgList>::run(this, std::forward<Sys>(s));
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
	template<typename... Cmpts>
	struct Each<TypeList<Cmpts * ...>> {
		static_assert(sizeof...(Cmpts) > 0);
		using CmptList = TypeList<std::remove_const_t<Cmpts>...>;
		static_assert(IsSet_v<CmptList>, "Componnents must be different");
		template<typename Sys>
		static void run(World* w, Sys&& s) {
			for (auto archetype : w->mngr.GetArchetypeWith<std::remove_const_t<Cmpts>...>()) {
				auto cmptsTupleVec = archetype->Locate<std::remove_const_t<Cmpts>...>();
				size_t num = archetype->Size();
				size_t chunkNum = archetype->ChunkNum();
				size_t chunkCapacity = archetype->ChunkCapacity();

				for (size_t i = 0; i < chunkNum; i++) {
					size_t J = std::min(chunkCapacity, num - (i * chunkCapacity));
					for (size_t j = 0; j < J; j++) {
						if constexpr (std::is_same_v<FuncTraits_Ret<Sys>, bool>) {
							if (!s((std::get<Find_v<CmptList, std::remove_const_t<Cmpts>>>(cmptsTupleVec[i]) + j)...))
								return;
						}
						else
							s((std::get<Find_v<CmptList, std::remove_const_t<Cmpts>>>(cmptsTupleVec[i]) + j)...);
					}
				}
			}
			w->mngr.RunCommands();
		}

		template<typename Sys>
		static void run(const World* w, Sys&& s) {
			static_assert((std::is_const_v<Cmpts> &&...),
				"arguments must be const <Component>*");
			run(const_cast<World*>(w), std::forward<Sys>(s));
		}
	};

	template<typename... Cmpts>
	struct ParallelEach<TypeList<Cmpts * ...>> {
		static_assert(sizeof...(Cmpts) > 0);
		using CmptList = TypeList<std::remove_const_t<Cmpts>...>;
		static_assert(IsSet_v<CmptList>, "Componnents must be different");
		template<typename Sys>
		static void run(World* w, Sys&& s) {
			tf::Taskflow taskflow;
			w->mngr.GenJob(&taskflow, std::forward<Sys>(s));
			w->executor.run(taskflow).wait();
			w->mngr.RunCommands();
		}

		template<typename Sys>
		static void run(const World* w, Sys&& s) {
			static_assert((std::is_const_v<Cmpts> &&...),
				"arguments must be const <Component>*");
			run(const_cast<World*>(w), std::forward<Sys>(s));
		}
	};
}
