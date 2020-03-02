#pragma once

#include "ArchetypeMngr.h"

namespace Ubpa {
	class World;

	class Entity final : private EntityData {
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

		template<typename... Cmpts>
		inline std::tuple<Cmpts*...> Attach() {
			// TODO: static_assert(Different_v<TypeList<Cmpts...>>);
			static_assert(sizeof...(Cmpts) > 0);
			assert(IsAlive());
			return archetype()->mngr->EntityAttach<Cmpts...>(this);
		}

		template<typename... Cmpts>
		inline void Detach() {
			// TODO: static_assert(Different_v<TypeList<Cmpts...>>);
			static_assert(sizeof...(Cmpts) > 0);
			assert(IsAlive());
			return archetype()->mngr->EntityDetach<Cmpts...>(this);
		}

		inline bool IsAlive() const noexcept { return archetype() != nullptr; }

		void Release() noexcept {
			assert(IsAlive());
			archetype()->mngr->Release(this);
		}

	private:
		friend class World;
	};

	static_assert(sizeof(Entity) == sizeof(EntityData) && std::is_base_of_v<EntityData, Entity>);
}
