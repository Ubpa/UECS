#pragma once

namespace Ubpa {
	template<typename System>
	void SystemMngr::RegisterOne() {
		lifecycleMap.emplace(TypeID<System>, SystemLifecycle{ &System::OnUpdate });
	}

	template<typename... Systems>
	void SystemMngr::Register() {
		(RegisterOne<Systems>(), ...);
	}

	template<typename System>
	bool SystemMngr::IsRegistered() const {
		return lifecycleMap.find(TypeID<System>) != lifecycleMap.end();
	}

	template<typename System>
	void SystemMngr::Deregister() noexcept {
		lifecycleMap.erase(TypeID<System>);
	}
}
