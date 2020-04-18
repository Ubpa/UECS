#include <UECS/World.h>

using namespace Ubpa;
using namespace std;

World::World() : sysMngr{&mngr}, mngr { &sysMngr, this } {}

void World::Update(bool dump) {
	tf::Taskflow taskflow;
	if (dump)
		taskflow.dump(std::cout);
	executor.run(taskflow).wait();
	mngr.RunCommand();
}
