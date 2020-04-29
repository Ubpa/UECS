#include <UECS/World.h>

#include <UECS/detail/CmptSysMngr.h>

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

	CmptSysMngr::Instance().GenSchedule(startSchedule, *this);
	startSchedule.GenTaskflow(startTaskflow);

	executor.run(startTaskflow).wait();
	
	mngr.RunCommands();
}

void World::Update() {
	updateSchedule.Clear();
	updateTaskflow.clear();

	CmptSysMngr::Instance().GenSchedule(updateSchedule, *this);
	updateSchedule.GenTaskflow(updateTaskflow);

	executor.run(updateTaskflow).wait();

	mngr.RunCommands();
}

void World::Stop() {
	stopSchedule.Clear();
	stopTaskflow.clear();

	CmptSysMngr::Instance().GenSchedule(stopSchedule, *this);
	stopSchedule.GenTaskflow(stopTaskflow);

	executor.run(stopTaskflow).wait();

	mngr.RunCommands();
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

void World::AddCommand(const std::function<void()>& command) {
	mngr.AddCommand(command);
}
