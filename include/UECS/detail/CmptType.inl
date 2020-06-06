#pragma once

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
