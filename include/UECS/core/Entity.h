#pragma once

#include "ArchetypeMngr.h"

namespace Ubpa {
	class World;

	class Entity : private EntityData {
	public:
		template<typename Cmpt, typename... Args>
		inline void Init(Args... args) {
			assert(IsAlive());
			archetype->Init<Cmpt>(idx, std::forward<Args>(args)...);
		}

		template<typename Cmpt>
		inline Cmpt* Get() {
			assert(IsAlive());
			return archetype()->At<Cmpt>(idx());
		}

		template<typename Cmpt, typename... Args>
		inline Cmpt* Add(Args&&... args) {
			assert(IsAlive());
			return archetype()->mngr->EntityAdd<Cmpt>(this, std::forward<Args>(args)...);
		}

		inline bool IsAlive() const noexcept { return archetype() != nullptr; }

	private:
		friend class World;
	};

	static_assert(sizeof(Entity) == sizeof(EntityData) && std::is_base_of_v<EntityData, Entity>);
}
