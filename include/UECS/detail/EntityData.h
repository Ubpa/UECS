#pragma once

namespace Ubpa {
	class Archetype;

	struct EntityData {
		EntityData(Archetype* archetype, size_t idx)
			: archetype{ archetype }, idx{ idx }, version{ version + 1 } {}
		~EntityData() { version += 1; }

		bool operator<(const EntityData& e) const noexcept {
			return archetype < e.archetype || (archetype == e.archetype &&
				(idx < e.idx || (idx == e.idx &&
					(version < e.version))));
		}
	protected:
		friend class EntityMngr;
		friend class EntityPtr;
		friend class EntityCPtr;

		size_t idx;
		size_t version;
		Archetype* archetype;
	};
}
