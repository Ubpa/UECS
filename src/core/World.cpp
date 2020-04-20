#include <UECS/World.h>

#include <UECS/detail/SystemMngr.h>

using namespace Ubpa;
using namespace std;

World::World()
	: mngr { this },
	startSchedule{&mngr},
	updateSchedule{ &mngr },
	stopSchedule{ &mngr }
{}

void World::Start() {
	startSchedule.Clear();
	startTaskflow.clear();

	SystemMngr::Instance().GenStartSchedule(startSchedule);
	startSchedule.GenTaskflow(startTaskflow);

	executor.run(startTaskflow).wait();
	
	mngr.RunCommand();
}

void World::Update() {
	updateSchedule.Clear();
	updateTaskflow.clear();

	SystemMngr::Instance().GenUpdateSchedule(updateSchedule);
	updateSchedule.GenTaskflow(updateTaskflow);

	executor.run(updateTaskflow).wait();

	mngr.RunCommand();
}

void World::Stop() {
	stopSchedule.Clear();
	stopTaskflow.clear();

	SystemMngr::Instance().GenStopSchedule(stopSchedule);
	stopSchedule.GenTaskflow(stopTaskflow);

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
