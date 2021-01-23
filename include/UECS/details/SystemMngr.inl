#pragma once

namespace Ubpa::UECS {
	template<typename... Systems>
	std::array<std::size_t, sizeof...(Systems)> SystemMngr::RegisterAndCreate() {
		std::array<std::size_t, sizeof...(Systems)> systemIDs = systemTraits.Register<Systems...>();
		std::apply([this](auto... IDs) {
			(Create(IDs), ...);
		}, systemIDs);
		return systemIDs;
	}

	template<typename... Systems>
	std::array<std::size_t, sizeof...(Systems)> SystemMngr::RegisterAndActivate() {
		std::array<std::size_t, sizeof...(Systems)> systemIDs = systemTraits.Register<Systems...>();
		std::apply([this](auto... IDs) {
			(Activate(IDs), ...);
		}, systemIDs);
		return systemIDs;
	}
}
