#include <UECS/detail/SystemMngr.h>

using namespace Ubpa;
using namespace std;

void SystemMngr::GenSchedule(SystemSchedule<SysType::OnStart>& schedule) {
	for (const auto& func : dynamicStartScheduleFuncs)
		func(schedule);

	for (const auto& func : staticStartScheduleFuncs)
		func(schedule);
}

void SystemMngr::GenSchedule(SystemSchedule<SysType::OnUpdate>& schedule) {
	for (const auto& func : dynamicUpdateScheduleFuncs)
		func(schedule);

	for (const auto& func : staticUpdateScheduleFuncs)
		func(schedule);
}

void SystemMngr::GenSchedule(SystemSchedule<SysType::OnStop>& schedule) {
	for (const auto& func : dynamicStopScheduleFuncs)
		func(schedule);

	for (const auto& func : staticStopScheduleFuncs)
		func(schedule);
}
