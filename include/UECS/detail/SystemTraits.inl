#pragma once

#include <nameof.hpp>

#include <UTemplate/Concept.h>

namespace Ubpa::UECS::detail {
	template<typename System>
	Concept(HaveOnCreate, static_cast<void(*)(World*)>(&System::OnCreate));
	template<typename System>
	Concept(HaveOnActivate, static_cast<void(*)(World*)>(&System::OnActivate));
	template<typename System>
	Concept(HaveOnUpdate, static_cast<void(*)(Schedule&)>(&System::OnUpdate));
	template<typename System>
	Concept(HaveOnDeactivate, static_cast<void(*)(World*)>(&System::OnDeactivate));
	template<typename System>
	Concept(HaveOnDestroy, static_cast<void(*)(World*)>(&System::OnDestroy));
	
	template<typename System>
	size_t Register(SystemTraits& traits) {
		size_t ID = traits.Register(std::string{ SystemTraits::StaticNameof<System>() });
		if constexpr (Require<HaveOnCreate, System>)
			traits.RegisterOnCreate(ID, std::function{ static_cast<void(*)(World*)>(&System::OnCreate) });
		if constexpr (Require<HaveOnActivate, System>)
			traits.RegisterOnActivate(ID, std::function{ static_cast<void(*)(World*)>(&System::OnActivate) });
		if constexpr (Require<HaveOnUpdate, System>)
			traits.RegisterOnUpdate(ID, static_cast<void(*)(Schedule&)>(&System::OnUpdate));
		if constexpr (Require<HaveOnDeactivate, System>)
			traits.RegisterOnDeactivate(ID, std::function{ static_cast<void(*)(World*)>(&System::OnDeactivate) });
		if constexpr (Require<HaveOnDeactivate, System>)
			traits.RegisterOnDestroy(ID, std::function{ static_cast<void(*)(World*)>(&System::OnDestroy) });
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
