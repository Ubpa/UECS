#pragma once

namespace Ubpa {
	class Entity final {
	public:
		size_t Idx() const noexcept { return idx; }
		size_t Version() const noexcept { return version; }
		friend bool operator==(const Entity& x, const Entity& y) noexcept {
			return x.idx == y.idx && x.version == y.version;
		}
		friend bool operator<(const Entity& x, const Entity& y) noexcept {
			return x.idx < y.idx || (x.idx == y.idx && x.version < y.version);
		}
	private:
		friend class EntityMngr;
		friend class Archetype;
		Entity(size_t idx, size_t version) : idx(idx), version(version) {}
		size_t idx;
		size_t version;
	};
}
