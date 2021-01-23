#pragma once

#include <UTemplate/Name.h>

namespace Ubpa::UECS::details {
	template<typename System>
	concept HaveOnCreate     = requires(World* w) { { System::OnCreate(w) }; };
	template<typename System>
	concept HaveOnActivate   = requires(World * w) { { System::OnActivate(w) }; };
	template<typename System>
	concept HaveOnUpdate     = requires(Schedule& schedule){ { System::OnUpdate(schedule) }; };
	template<typename System>
	concept HaveOnDeactivate = requires(World * w) { { System::OnDeactivate(w) }; };
	template<typename System>
	concept HaveOnDestroy    = requires(World * w) { { System::OnDestroy(w) }; };
	
	template<typename System>
	std::size_t Register(SystemTraits& traits) {
		std::size_t ID = traits.Register(std::string{ SystemTraits::StaticNameof<System>() });
		if constexpr (HaveOnCreate<System>)
			traits.RegisterOnCreate    (ID, static_cast<SystemTraits::OnCreate*    >(&System::OnCreate     ));
		if constexpr (HaveOnActivate<System>)
			traits.RegisterOnActivate  (ID, static_cast<SystemTraits::OnActivate*  >(&System::OnActivate  ));
		if constexpr (HaveOnUpdate<System>)
			traits.RegisterOnUpdate    (ID, static_cast<SystemTraits::OnUpdate*    >(&System::OnUpdate    ));
		if constexpr (HaveOnDeactivate<System>)
			traits.RegisterOnDeactivate(ID, static_cast<SystemTraits::OnDeactivate*>(&System::OnDeactivate));
		if constexpr (HaveOnDeactivate<System>)
			traits.RegisterOnDestroy   (ID, static_cast<SystemTraits::OnDestroy*   >(&System::OnDestroy   ));
		return ID;
	}
}

namespace Ubpa::UECS {
	template<typename... Systems>
	std::array<std::size_t, sizeof...(Systems)> SystemTraits::Register() {
		return { details::Register<Systems>(*this)... };
	}

	template<typename System>
	std::string_view SystemTraits::StaticNameof() noexcept {
		return type_name<System>().View();
	}
}
