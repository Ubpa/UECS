#pragma once

namespace Ubpa {
	class Archetype;

	struct EntityBase {
		Archetype* archetype{nullptr};
		size_t idx{ static_cast<size_t>(-1) };

		bool operator<(const EntityBase& e) const noexcept {
			return archetype < e.archetype ||
				(archetype == e.archetype && idx < e.idx);
		}

	private:
		friend class ArchetypeMngr;
	};
}
