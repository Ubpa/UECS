#pragma once

#include "details/Util.hpp"

#include <compare>
#include <type_traits>

namespace Ubpa::UECS {
	// index + version
	struct Entity {
		std::uint64_t index;
		std::uint64_t version;

		static constexpr Entity Invalid() noexcept
		{ return { .index = static_cast<std::uint64_t>(-1), .version = static_cast<std::uint64_t>(-1) }; }

		constexpr bool Valid() const noexcept { return index != static_cast<std::uint64_t>(-1); }
		constexpr auto operator<=>(const Entity&) const = default;
	};
	static_assert(std::is_trivial_v<Entity>);
}
