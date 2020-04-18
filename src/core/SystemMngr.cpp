#include <UECS/detail/SystemMngr.h>

using namespace Ubpa;
using namespace std;

SystemMngr::SystemMngr(ArchetypeMngr* archetypeMngr)
	: schedule(archetypeMngr)
{
}

void SystemMngr::GenTaskflow(tf::Taskflow& taskflow) {
	assert(taskflow.empty());

	schedule.Clear();

	for (auto& func : dynamicScheduleFuncs)
		func(schedule);

	for (const auto& func : staticScheduleFuncs)
		func(schedule);

	schedule.GenTaskflow(taskflow);
}
