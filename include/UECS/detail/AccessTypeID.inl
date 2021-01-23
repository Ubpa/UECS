#pragma once

namespace std {
	template<typename T>
	struct hash;
}

template<>
struct std::hash<Ubpa::UECS::AccessTypeID> {
	constexpr std::size_t operator()(const Ubpa::UECS::AccessTypeID& id) const noexcept {
		return id.GetValue();
	}
};
