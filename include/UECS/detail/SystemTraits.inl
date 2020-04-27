#pragma once

#include <_deps/nameof.hpp>

namespace Ubpa {
	template<typename Cmpt, SysType type, typename>
	constexpr auto GetSys() noexcept {
		if constexpr (type == SysType::OnStart)
			return &Cmpt::OnStart;
		else if constexpr (type == SysType::OnUpdate)
			return &Cmpt::OnUpdate;
		else // if constexpr (type == SysType::Stop)
			return &Cmpt::OnStop;
	}

	template<typename Cmpt, SysType type, typename>
	constexpr std::string_view DefaultSysName() noexcept {
		return nameof::nameof_type<decltype(GetSys<Cmpt, type>())>();
	}
}
