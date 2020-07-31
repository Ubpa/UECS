#pragma once

namespace Ubpa::UECS {
	template<typename DerivedSystem>
	void SystemMngr::RegisterOne() {
		static_assert(std::is_base_of_v<System, DerivedSystem> && std::is_constructible_v<DerivedSystem, World*, std::string>,
			"<DerivedSystem> must be derived form System and constructible with World* and std::string");
		auto derivedSystem = new DerivedSystem{ world, std::string{ nameof::nameof_type<DerivedSystem>() } };
		Register(std::unique_ptr<System>{ static_cast<System*>(derivedSystem) });
	}

	template<typename... DerivedSystems>
	void SystemMngr::Register() {
		(RegisterOne<DerivedSystems>(), ...);
	}

	template<typename DerivedSystem>
	bool SystemMngr::IsRegistered() const {
		return IsRegistered(nameof::nameof_type<DerivedSystem>());
	}

	template<typename DerivedSystem>
	void DeregisterOne() {
		Deregister(nameof::nameof_type<DerivedSystem>());
	}

	template<typename... DerivedSystems>
	void SystemMngr::Deregister() noexcept {
		(DeregisterOne<DerivedSystems>(), ...);
	}
}
