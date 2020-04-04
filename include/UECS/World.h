#pragma once

#include "Entity.h"
#include "detail/SystemMngr.h"

#include <UTemplate/Func.h>

#include <thread>

namespace Ubpa::detail::World_ {
	template<typename Args>
	struct Each;
	template<typename Args>
	struct ParallelEach;
}

// no any lock !
namespace Ubpa {
	class World {
	public:
		World();

		template<typename... Cmpts>
		inline std::tuple<Entity*, Cmpts*...> CreateEntity();

		void Update(bool dump = false);

		// s must be a callable object and it's argument-list isn't empty
		template<typename Sys>
		inline void Each(Sys&& s);

		// s must be a callable object and it's argument-list isn't empty
		template<typename Sys>
		inline void ParallelEach(Sys&& s);

	private:
		template<typename ArgList>
		friend struct detail::World_::Each;
		template<typename ArgList>
		friend struct detail::World_::ParallelEach;
		SystemMngr sysMngr;
		tf::Executor executor;
		ArchetypeMngr mngr;
	};
}

#include "detail/World.inl"
