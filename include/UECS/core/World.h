#pragma once

#include "ArchetypeMngr.h"
#include "Entity.h"
#include "pool.h"

#include <UTemplate/FuncTraits.h>

#include <tuple>

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
		World() : mngr(new ArchetypeMngr(this)) {}
		~World() { delete mngr; }

		template<typename... Cmpts>
		inline std::tuple<Entity*, Cmpts*...> CreateEntity();

		// s must be a callable object and it's argument-list isn't empty
		template<typename ArgList>
		friend struct detail::World_::Each;
		template<typename Sys>
		inline void Each(Sys&& s) {
			detail::World_::Each<typename FuncTraits<Sys>::ArgList>::run(this, std::forward<Sys>(s));
		}

		// s must be a callable object and it's argument-list isn't empty
		template<typename ArgList>
		friend struct detail::World_::ParallelEach;
		template<typename Sys>
		inline void ParallelEach(Sys&& s) {
			detail::World_::ParallelEach<typename FuncTraits<Sys>::ArgList>::run(this, std::forward<Sys>(s));
		}

	private:
		ArchetypeMngr* mngr;
	};
}

#include "detail/World.inl"
