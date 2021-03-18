#pragma once

#include <UTemplate/Name.hpp>

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
	Name Register(SystemTraits& traits) {
		Name name = traits.Register(type_name<System>().View());
		if constexpr (HaveOnCreate<System>)
			traits.RegisterOnCreate(name, static_cast<SystemTraits::OnCreate*>(&System::OnCreate));
		if constexpr (HaveOnActivate<System>)
			traits.RegisterOnActivate(name, static_cast<SystemTraits::OnActivate*>(&System::OnActivate));
		if constexpr (HaveOnUpdate<System>)
			traits.RegisterOnUpdate(name, static_cast<SystemTraits::OnUpdate*>(&System::OnUpdate));
		if constexpr (HaveOnDeactivate<System>)
			traits.RegisterOnDeactivate(name, static_cast<SystemTraits::OnDeactivate*>(&System::OnDeactivate));
		if constexpr (HaveOnDeactivate<System>)
			traits.RegisterOnDestroy(name, static_cast<SystemTraits::OnDestroy*>(&System::OnDestroy));
		return name;
	}
}

namespace Ubpa::UECS {
	template<typename Sys>
	constexpr Name SystemTraits::Nameof() noexcept {
		static_assert(std::is_same_v<std::remove_cvref_t<Sys>, Sys>);
		constexpr auto v = type_name<Sys>().View();
		constexpr Name n{ v };
		return n;
	}

	template<typename... Systems>
	std::array<Name, sizeof...(Systems)> SystemTraits::Register() {
		return { details::Register<Systems>(*this)... };
	}

	template<typename System>
	bool SystemTraits::IsRegistered() const {
		return type_name<System>().View();
	}
}
