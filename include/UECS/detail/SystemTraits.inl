#pragma once

#include <nameof.hpp>

#include <UTemplate/Concept.h>

namespace Ubpa::UECS::detail {
	template<typename System>
	Concept(HaveOnCreate,     static_cast<SystemTraits::OnCreate*    >(&System::OnCreate    ));
	template<typename System>
	Concept(HaveOnActivate,   static_cast<SystemTraits::OnActivate*  >(&System::OnActivate  ));
	template<typename System>
	Concept(HaveOnUpdate,     static_cast<SystemTraits::OnUpdate*    >(&System::OnUpdate    ));
	template<typename System>
	Concept(HaveOnDeactivate, static_cast<SystemTraits::OnDeactivate*>(&System::OnDeactivate));
	template<typename System>
	Concept(HaveOnDestroy,    static_cast<SystemTraits::OnDestroy*   >(&System::OnDestroy   ));
	
	template<typename System>
	size_t Register(SystemTraits& traits) {
		size_t ID = traits.Register(std::string{ SystemTraits::StaticNameof<System>() });
		if constexpr (Require<HaveOnCreate, System>)
			traits.RegisterOnCreate    (ID, static_cast<SystemTraits::OnCreate*   >(&System::OnCreate     ));
		if constexpr (Require<HaveOnActivate, System>)
			traits.RegisterOnActivate  (ID, static_cast<SystemTraits::OnActivate*  >(&System::OnActivate  ));
		if constexpr (Require<HaveOnUpdate, System>)
			traits.RegisterOnUpdate    (ID, static_cast<SystemTraits::OnUpdate*    >(&System::OnUpdate    ));
		if constexpr (Require<HaveOnDeactivate, System>)
			traits.RegisterOnDeactivate(ID, static_cast<SystemTraits::OnDeactivate*>(&System::OnDeactivate));
		if constexpr (Require<HaveOnDeactivate, System>)
			traits.RegisterOnDestroy   (ID, static_cast<SystemTraits::OnDestroy*   >(&System::OnDestroy   ));
		return ID;
	}
}

namespace Ubpa::UECS {
	template<typename... Systems>
	std::array<size_t, sizeof...(Systems)> SystemTraits::Register() {
		return { detail::Register<Systems>(*this)... };
	}

	template<typename System>
	std::string_view SystemTraits::StaticNameof() noexcept {
		return nameof::nameof_type<System>();
	}
}
