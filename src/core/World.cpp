#include <UECS/World.h>

using namespace Ubpa;
using namespace std;

World::World() : sysMngr{&mngr}, mngr { &sysMngr, this } {}

void World::Update(bool dump) {
	map<SystemMngr::ScheduleType, tf::Taskflow> type2tf;
	sysMngr.GenTaskflow(type2tf);
	for (auto& [type, taskflow] : type2tf) {
		if (dump)
			taskflow.dump(std::cout);
		executor.run(taskflow).wait();
	}
}

void World::RunCommand() {
	mngr.RunCommand();
}

