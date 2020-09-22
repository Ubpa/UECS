#pragma once

#include <_deps/nameof.hpp>

namespace Ubpa::UECS {
	template<typename System>
	size_t SystemMngr::GetIndex() const {
		return GetIndex(nameof::nameof_type<System>());
	}

	template<typename System>
	size_t SystemMngr::Register() {
		return Register(std::string{ nameof::nameof_type<System>() }, &System::OnUpdate);
	}
}
