#pragma once

namespace Ubpa {
	constexpr bool operator<(CmptType x, CmptType y) noexcept {
		return x.HashCode() < y.HashCode();
	}
	constexpr bool operator==(CmptType x, CmptType y) noexcept {
		return x.HashCode() == y.HashCode();
	}
}

namespace std {
	template<typename T>
	struct hash;

	template<>
	struct hash<Ubpa::CmptType> {
		constexpr size_t operator()(Ubpa::CmptType t) const noexcept {
			return t.HashCode();
		}
	};
}
