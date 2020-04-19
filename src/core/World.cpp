#include <UECS/World.h>

using namespace Ubpa;
using namespace std;

World::World() : sysMngr{&mngr}, mngr { &sysMngr, this } {}

void World::Start() {
	startTaskflow.clear();
	sysMngr.GenStartTaskflow(startTaskflow);
	executor.run(startTaskflow).wait();
	mngr.RunCommand();
}

void World::Update() {
	updateTaskflow.clear();
	sysMngr.GenUpdateTaskflow(updateTaskflow);
	executor.run(updateTaskflow).wait();
	mngr.RunCommand();
}

void World::Stop() {
	stopTaskflow.clear();
	sysMngr.GenStopTaskflow(stopTaskflow);
	executor.run(stopTaskflow).wait();
	mngr.RunCommand();
}

string World::DumpStartTaskflow() const {
	return startTaskflow.dump();
}

string World::DumpUpdateTaskflow() const {
	return updateTaskflow.dump();
}

string World::DumpStopTaskflow() const {
	return stopTaskflow.dump();
}
