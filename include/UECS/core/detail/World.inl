#pragma once

namespace Ubpa::detail::World_ {
	template<typename... Cmpts>
	struct Each<TypeList<Cmpts * ...>> {
		using CmptList = TypeList<Cmpts...>;
		template<typename Sys>
		static void run(World* w, Sys&& s) {
			for (auto archeType : w->mngr.GetArcheTypeWith<Cmpts...>()) {
				std::array<void*, sizeof...(Cmpts)> arr = { archeType->Get<Cmpts>() ... };
				for (size_t i = 0; i < archeType->Size(); i++)
					s((reinterpret_cast<Cmpts*>(arr[Find_v<CmptList, Cmpts>]) + i)...);
			}
		}
	};
}

namespace Ubpa {
	template<typename... Cmpts>
	Entity* World::CreateEntityWith() {
		detail::ArcheType& c = mngr.GetArcheTypeOf<Cmpts...>();
		size_t id = c.CreateEntity<Cmpts...>();

		auto entity = entities.request();
		entity->isAlive = true;
		entity->ID = id;
		entity->archeType = &c;

		atid2e[std::make_tuple(&c, id)] = entity;
		e2atid[entity] = std::make_tuple(&c, id);

		return entity;
	}
}
