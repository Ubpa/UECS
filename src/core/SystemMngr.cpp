#include <UECS/SystemMngr.h>

#include <UECS/detail/CmptSysMngr.h>

using namespace Ubpa;
using namespace std;

SystemMngr::SystemMngr(EntityMngr* entityMngr)
	: startRegistrar{ entityMngr },
	updateRegistrar{ entityMngr },
	stopRegistrar{ entityMngr }
{
}

void SystemMngr::Start() {
	startRegistrar.schedule.Clear();
	startJobGraph.clear();

	CmptSysMngr::Instance().GenSchedule(startRegistrar, *this);
	startRegistrar.schedule.GenJobGraph(startJobGraph);

	executor.run(startJobGraph).wait();
}

void SystemMngr::Update() {
	updateRegistrar.schedule.Clear();
	updateJobGraph.clear();

	CmptSysMngr::Instance().GenSchedule(updateRegistrar, *this);
	updateRegistrar.schedule.GenJobGraph(updateJobGraph);

	executor.run(updateJobGraph).wait();
}

void SystemMngr::Stop() {
	stopRegistrar.schedule.Clear();
	stopJobGraph.clear();

	CmptSysMngr::Instance().GenSchedule(stopRegistrar, *this);
	stopRegistrar.schedule.GenJobGraph(stopJobGraph);

	executor.run(stopJobGraph).wait();
}

string SystemMngr::DumpStartTaskflow() const {
	return startJobGraph.dump();
}

string SystemMngr::DumpUpdateTaskflow() const {
	return updateJobGraph.dump();
}

string SystemMngr::DumpStopTaskflow() const {
	return stopJobGraph.dump();
}
