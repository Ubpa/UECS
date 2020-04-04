#include <UECS/World.h>

using namespace Ubpa;

World::World() : mngr{ &sysMngr, this } {}

void World::Update(bool dump) {
	SystemMngr::Table table;
	sysMngr.GenTaskflow(table);
	if (dump)
		table.finalSys.dump(std::cout);
	executor.run(table.finalSys).wait();
}
