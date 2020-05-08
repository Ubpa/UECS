#pragma once

#include "../CmptRegistrar.h"

#include <UTemplate/Func.h>

namespace Ubpa {
	template<typename... Cmpts>
	std::tuple<EntityPtr, Cmpts*...> World::CreateEntity() {
		assert("World::CreateEntity: <Cmpts> are unregistered"
			&& CmptRegistrar::Instance().template IsRegistered<Cmpts...>());
		auto rst = entityMngr.CreateEntity<Cmpts...>();
		return { EntityPtr{reinterpret_cast<Entity*>(std::get<0>(rst))},
			std::get<1 + Find_v<TypeList<Cmpts...>, Cmpts>>(rst)... };
	}
}
