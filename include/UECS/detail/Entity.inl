#pragma once

#include "../CmptRegistrar.h"

namespace Ubpa {
	template<typename Cmpt>
	Cmpt* Entity::Get() {
		return archetype->At<Cmpt>(idx);
	}

	template<typename Cmpt>
	const Cmpt* Entity::Get() const { return const_cast<Entity*>(this)->Get(); }

	template<typename... Cmpts>
	std::tuple<Cmpts *...> Entity::Attach() {
		assert("Entity::Attach: <Cmpts> are unregistered" &&
			CmptRegistrar::Instance().template IsRegistered<Cmpts...>());
		return archetype->mngr->EntityAttach<Cmpts...>(this);
	}

	template<typename Cmpt, typename... Args>
	Cmpt* Entity::AssignAttach(Args... args) {
		assert("Entity::AssignAttach: <Cmpt> is unregistered" &&
			CmptRegistrar::Instance().template IsRegistered<Cmpt>());
		return archetype->mngr->EntityAssignAttach<Cmpt>(this, std::forward<Args>(args)...);
	}

	template<typename Cmpt>
	inline Cmpt* Entity::GetOrAttach() {
		Cmpt* cmpt = archetype->At<Cmpt>(idx);
		if (!cmpt)
			std::tie(cmpt) = Attach<Cmpt>();
		return cmpt;
	}

	template<typename... Cmpts>
	void Entity::Detach() {
		static_assert(sizeof...(Cmpts) > 0);
		static_assert(IsSet_v<TypeList<Cmpts...>>, "Componnents must be different");
		return archetype->mngr->EntityDetach<Cmpts...>(this);
	}
}
