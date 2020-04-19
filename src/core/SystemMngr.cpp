#include <UECS/detail/SystemMngr.h>

using namespace Ubpa;
using namespace std;

SystemMngr::SystemMngr(ArchetypeMngr* archetypeMngr)
	: startSchedule(archetypeMngr),
	updateSchedule(archetypeMngr),
	stopSchedule(archetypeMngr)
{
}

void SystemMngr::GenStartTaskflow(tf::Taskflow& taskflow) {
	assert(taskflow.empty());

	startSchedule.Clear();

	for (const auto& func : dynamicStartScheduleFuncs)
		func(startSchedule);

	for (const auto& func : staticStartScheduleFuncs)
		func(startSchedule);

	startSchedule.GenTaskflow(taskflow);
}

void SystemMngr::GenUpdateTaskflow(tf::Taskflow& taskflow) {
	assert(taskflow.empty());

	updateSchedule.Clear();

	for (const auto& func : dynamicUpdateScheduleFuncs)
		func(updateSchedule);

	for (const auto& func : staticUpdateScheduleFuncs)
		func(updateSchedule);

	updateSchedule.GenTaskflow(taskflow);
}

void SystemMngr::GenStopTaskflow(tf::Taskflow& taskflow) {
	assert(taskflow.empty());

	stopSchedule.Clear();

	for (const auto& func : dynamicStopScheduleFuncs)
		func(stopSchedule);

	for (const auto& func : staticStopScheduleFuncs)
		func(stopSchedule);

	stopSchedule.GenTaskflow(taskflow);
}
