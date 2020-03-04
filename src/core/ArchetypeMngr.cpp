#pragma once

#include <UECS/core/ArchetypeMngr.h>

using namespace Ubpa;

void ArchetypeMngr::Release(EntityBase* e) {
	auto archetype = e->archetype;
	auto idx = e->idx;
	entityPool.recycle(e);

	auto [movedEntityIdx, pairs] = archetype->Erase(idx);

	if (movedEntityIdx != static_cast<size_t>(-1)) {
		auto target = ai2e.find({ archetype, movedEntityIdx });
		EntityBase* movedEntity = target->second;
		for (auto p : pairs)
			movedEntity->MoveCmpt(p.first, p.second);
		movedEntity->idx = idx;
		ai2e[{archetype, idx}] = movedEntity;
		ai2e.erase(target);
	}

	if (archetype->Size() == 0 && archetype->CmptNum() != 0) {
		ids.erase(archetype->id);
		id2a.erase(archetype->id);
		delete archetype;
	}

	archetype = nullptr;
	idx = static_cast<size_t>(-1);
}
