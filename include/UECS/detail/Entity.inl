#pragma once

#include "SystemMngr.h"

namespace Ubpa {
	template<typename Cmpt>
	inline Cmpt* Entity::Get() {
		assert(IsAlive());
		return archetype->At<Cmpt>(idx);
	}

	const std::vector<std::tuple<void*, size_t>> Entity::Components() const {
		return archetype->Components(idx);
	}

	template<typename... Cmpts>
	inline std::tuple<Cmpts *...> Entity::Attach() {
		static_assert(sizeof...(Cmpts) > 0);
		static_assert(IsSet_v<TypeList<Cmpts...>>, "Componnents must be different");
		(CmptMngr::Instance().Regist<Cmpts>(), ...);
		assert(IsAlive());
		(archetype->sysmngr->Regist<Cmpts>(), ...);
		return archetype->mngr->EntityAttach<Cmpts...>(this);
	}

	template<typename Cmpt>
	inline Cmpt* Entity::GetOrAttach() {
		assert(IsAlive());
		Cmpt* cmpt = archetype->At<Cmpt>(idx);
		if (!cmpt)
			std::tie(cmpt) = Attach<Cmpt>();
		return cmpt;
	}

	template<typename... Cmpts>
	void Entity::Detach() {
		static_assert(sizeof...(Cmpts) > 0);
		static_assert(IsSet_v<TypeList<Cmpts...>>, "Componnents must be different");
		assert(IsAlive());
		return archetype->mngr->EntityDetach<Cmpts...>(this);
	}

	inline void Entity::Release() noexcept {
		assert(IsAlive());
		archetype->mngr->Release(this);
	}

	inline bool Entity::IsAlive() const noexcept {
		return archetype != nullptr;
	}
}
