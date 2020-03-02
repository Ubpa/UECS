#pragma once

#include "ArchetypeMngr.h"
#include "Entity.h"
#include "Pool.h"

#include <UTemplate/FuncTraits.h>

#include <tuple>

namespace Ubpa::detail::World_ {
	template<typename Args>
	struct Each;
}

namespace Ubpa {
	class World {
	public:
		World() : mngr(new ArchetypeMngr(this)) {}

		template<typename... Cmpts>
		inline Entity* CreateEntityWith();

		template<typename ArgList>
		friend struct detail::World_::Each;
		template<typename Sys>
		inline void Each(Sys&& s) {
			detail::World_::Each<typename FuncTraits<Sys>::ArgList>::run(this, std::forward<Sys>(s));
		}

	private:
		ArchetypeMngr* mngr;
	};
}

#include "detail/World.inl"
