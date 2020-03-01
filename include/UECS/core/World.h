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
		World() : mngr(this) {}

		template<typename... Cmpts>
		inline Entity* CreateEntityWith();

		void Delete(Entity* entity);

		template<typename ArgList>
		friend struct detail::World_::Each;
		template<typename Sys>
		inline void Each(Sys&& s) {
			detail::World_::Each<typename FuncTraits<Sys>::ArgList>::run(this, std::forward<Sys>(s));
		}

	private:
		detail::ArcheTypeMngr mngr;
		Pool<Entity> entities;
		std::map<Entity*, std::tuple<detail::ArcheType*, size_t>> e2atid;
		std::map<std::tuple<detail::ArcheType*, size_t>, Entity*> atid2e;
	};
}

#include "detail/World.inl"
