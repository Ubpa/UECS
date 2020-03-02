#pragma once

#include <UECS/core/ArchetypeMngr.h>

using namespace Ubpa;

void ArchetypeMngr::Release(EntityData* e) {
	Archetype* archetype = e->archetype();
	size_t movedEntityIdx = archetype->Erase(e->idx());

	auto target = d2p.find({ archetype, movedEntityIdx });
	if (movedEntityIdx != static_cast<size_t>(-1)) {
		EntityData* movedEntity = target->second;
		movedEntity->idx() = e->idx();
		d2p[*e] = movedEntity;
	}
	d2p.erase(target);

	if (archetype->Size() == 0 && archetype->CmptNum() != 0) {
		ids.erase(archetype->id);
		id2a.erase(archetype->id);
		delete archetype;
	}

	e->archetype() = nullptr;
	e->idx() = static_cast<size_t>(-1);

	entityPool.recycle(e);
}
