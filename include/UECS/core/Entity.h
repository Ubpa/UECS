#pragma once

#include "detail/ArchetypeMngr.h"

namespace Ubpa {
	class Entity final : private EntityBase {
	public:
		template<typename Cmpt>
		inline Cmpt* Get();

		template<typename... Cmpts>
		inline std::tuple<Cmpts *...> Attach();

		template<typename Cmpt>
		inline Cmpt* GetOrAttach();

		template<typename... Cmpts>
		inline void Detach();

		inline bool IsAlive() const noexcept;

		inline void Release() noexcept;
	};

	static_assert(sizeof(Entity) == sizeof(EntityBase) && std::is_base_of_v<EntityBase, Entity>);
}

#include "detail/Entity.inl"
