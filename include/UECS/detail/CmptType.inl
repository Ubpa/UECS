#pragma once

namespace std {
	template<typename T>
	struct hash;

	template<>
	struct hash<Ubpa::UECS::CmptType> {
		constexpr size_t operator()(const Ubpa::UECS::CmptType& t) const noexcept {
			return t.HashCode();
		}
	};

	template<>
	struct hash<Ubpa::UECS::CmptAccessType> {
		constexpr size_t operator()(const Ubpa::UECS::CmptAccessType& t) const noexcept {
			return t.HashCode();
		}
	};
}
