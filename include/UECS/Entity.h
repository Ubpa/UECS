#pragma once

#include "details/Util.h"

namespace Ubpa::UECS {
	// index + version
	class Entity {
	public:
		constexpr Entity(std::size_t idx, std::size_t version) noexcept : idx(idx), version(version) {}
		constexpr Entity() noexcept : Entity{ Invalid() } {}
		constexpr std::size_t Idx() const noexcept { return idx; }
		constexpr std::size_t Version() const noexcept { return version; }
		static constexpr Entity Invalid() noexcept { return { static_cast<std::size_t>(-1),static_cast<std::size_t>(-1) }; }
		constexpr bool Valid() const noexcept { return idx != static_cast<std::size_t>(-1); }
		constexpr bool operator==(const Entity& rhs) const noexcept {
			return idx == rhs.idx && version == rhs.version;
		}
		constexpr bool operator<(const Entity& rhs) const noexcept {
			return idx < rhs.idx || (idx == rhs.idx && version < rhs.version);
		}
	private:
		friend class EntityMngr;
		std::size_t idx;
		std::size_t version;
	};
}
