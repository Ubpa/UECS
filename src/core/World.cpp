#include <UECS/World.h>

#include <UECS/detail/CmptSysMngr.h>

using namespace Ubpa;
using namespace std;

World::World()
	: mngr { this },
	startRegistrar{ &mngr },
	updateRegistrar{ &mngr },
	stopRegistrar{ &mngr }
{
}

void World::Start() {
	startRegistrar.schedule.Clear();
	startJobGraph.clear();

	CmptSysMngr::Instance().GenSchedule(startRegistrar, *this);
	startRegistrar.schedule.GenJobGraph(startJobGraph);

	executor.run(startJobGraph).wait();
	
	mngr.RunCommands();
}

void World::Update() {
	updateRegistrar.schedule.Clear();
	updateJobGraph.clear();

	CmptSysMngr::Instance().GenSchedule(updateRegistrar, *this);
	updateRegistrar.schedule.GenJobGraph(updateJobGraph);

	executor.run(updateJobGraph).wait();

	mngr.RunCommands();
}

void World::Stop() {
	stopRegistrar.schedule.Clear();
	stopJobGraph.clear();

	CmptSysMngr::Instance().GenSchedule(stopRegistrar, *this);
	stopRegistrar.schedule.GenJobGraph(stopJobGraph);

	executor.run(stopJobGraph).wait();

	mngr.RunCommands();
}

string World::DumpStartTaskflow() const {
	return startJobGraph.dump();
}

string World::DumpUpdateTaskflow() const {
	return updateJobGraph.dump();
}

string World::DumpStopTaskflow() const {
	return stopJobGraph.dump();
}

void World::AddCommand(const std::function<void()>& command) {
	mngr.AddCommand(command);
}
