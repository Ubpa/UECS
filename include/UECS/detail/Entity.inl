#pragma once

#include "../CmptRegister.h"

namespace Ubpa {
	template<typename Cmpt>
	Cmpt* Entity::Get() {
		assert(IsAlive());
		return archetype->At<Cmpt>(idx);
	}

	template<typename Cmpt>
	const Cmpt* Entity::Get() const { return const_cast<Entity*>(this)->Get(); }

	template<typename... Cmpts>
	std::tuple<Cmpts *...> Entity::Attach() {
		static_assert(sizeof...(Cmpts) > 0);
		static_assert(IsSet_v<TypeList<Cmpts...>>, "Componnents must be different");
		assert("[ERROR] hasn't registed <Cmpts>" &&
			CmptRegister::Instance().template IsRegisted<Cmpts...>());
		assert(IsAlive());
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
}
