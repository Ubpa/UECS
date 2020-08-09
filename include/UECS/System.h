#pragma once

#include "Schedule.h"

namespace Ubpa::UECS {
	class World;

	class System {
	public:
		System(World* world, std::string name) noexcept : world{ world }, name{ name } {}

		World* GetWorld() const noexcept { return world; }
		const std::string& GetName() const noexcept { return name; }

		virtual void OnUpdate(Schedule&) = 0;
	private:
		World* world;
		std::string name;
	};
}
