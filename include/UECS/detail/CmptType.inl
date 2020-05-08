#pragma once

namespace Ubpa {
	inline constexpr bool operator<(const CmptType& x, const CmptType& y) noexcept {
		return x.HashCode() < y.HashCode();
	}
	inline constexpr bool operator==(const CmptType& x, const CmptType& y) noexcept {
		return x.HashCode() == y.HashCode();
	}
}

namespace std {
	template<typename T>
	struct hash;

	template<>
	struct hash<Ubpa::CmptType> {
		constexpr size_t operator()(const Ubpa::CmptType& t) const noexcept {
			return t.HashCode();
		}
	};
}
