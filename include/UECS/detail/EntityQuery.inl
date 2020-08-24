#pragma once

namespace std {
	template<typename T>
	struct hash;
	template<>
	struct hash<Ubpa::UECS::EntityQuery> {
		size_t operator()(const Ubpa::UECS::EntityQuery& query) const noexcept {
			return query.HashCode();
		}
	};
}
