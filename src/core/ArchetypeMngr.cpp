#pragma once

#include <UECS/core/ArchetypeMngr.h>

using namespace Ubpa;

void ArchetypeMngr::Release(EntityData* e) {
	size_t movedEntityIdx = e->archetype()->Erase(e->idx());

	auto target = d2p.find({ e->archetype(), movedEntityIdx });
	EntityData* movedEntity = target->second;
	d2p.erase(target);

	movedEntity->idx() = e->idx();
	d2p[*e] = movedEntity;

	entityPool.recycle(e);
}
