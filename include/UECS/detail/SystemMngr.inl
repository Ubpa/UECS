#pragma once

namespace Ubpa::UECS {
	template<typename System>
	void SystemMngr::RegisterOne() {
		Register(std::string{ nameof::nameof_type<System>() }, &System::OnUpdate);
	}

	template<typename... Systems>
	void SystemMngr::Register() {
		(RegisterOne<Systems>(), ...);
	}

	template<typename System>
	bool SystemMngr::IsRegistered() const {
		return IsRegistered(nameof::nameof_type<System>());
	}

	template<typename System>
	void DeregisterOne() {
		Deregister(std::string{ nameof::nameof_type<System>() });
	}

	template<typename... Systems>
	void SystemMngr::Deregister() noexcept {
		(DeregisterOne<Systems>(), ...);
	}
}
