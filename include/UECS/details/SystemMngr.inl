#pragma once

namespace Ubpa::UECS {
	template<typename... Systems>
	void SystemMngr::Create() { (Create(SystemTraits::Nameof<Systems>()), ...); }

	template<typename... Systems>
	void SystemMngr::Activate() { (Activate(SystemTraits::Nameof<Systems>()), ...); }

	template<typename... Systems>
	void SystemMngr::Deactivate() { (Deactivate(SystemTraits::Nameof<Systems>()), ...); }

	template<typename... Systems>
	void SystemMngr::Destroy() { (Destroy(SystemTraits::Nameof<Systems>()), ...); }

	template<typename System>
	bool SystemMngr::IsAlive() const { return IsAlive(SystemTraits::Nameof<System>()); }

	template<typename System>
	bool SystemMngr::IsActive() const { return IsActive(SystemTraits::Nameof<System>()); }

	template<typename... Systems>
	std::array<Name, sizeof...(Systems)> SystemMngr::RegisterAndCreate() {
		std::array<Name, sizeof...(Systems)> names = systemTraits.Register<Systems...>();
		std::apply([this](auto... names) {
			(Create(names), ...);
		}, names);
		return names;
	}

	template<typename... Systems>
	std::array<Name, sizeof...(Systems)> SystemMngr::RegisterAndActivate() {
		std::array<Name, sizeof...(Systems)> names = systemTraits.Register<Systems...>();
		std::apply([this](auto... names) {
			(Activate(names), ...);
		}, names);
		return names;
	}
}
