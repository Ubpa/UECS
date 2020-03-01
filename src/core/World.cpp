#include <UECS/core/World.h>

using namespace Ubpa;

void World::Delete(Entity* entity) {
	size_t movedID = entity->archeType->Delete(entity->ID);

	auto target = atid2e.find(std::make_tuple(entity->archeType, movedID));
	auto movedE = target->second;
	atid2e.erase(target);
	atid2e[std::make_tuple(entity->archeType, entity->ID)] = movedE;
	e2atid[movedE] = std::make_tuple(entity->archeType, entity->ID);
	movedE->ID = entity->ID;

	e2atid.erase(entity);
	atid2e.erase(std::make_tuple(entity->archeType, entity->ID));
	entity->isAlive = false;
	entities.recycle(entity);
}
