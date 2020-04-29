#include <UECS/detail/CmptSysMngr.h>

using namespace Ubpa;
using namespace std;

void CmptSysMngr::GenSchedule(SystemSchedule<SysType::OnStart>& schedule, const SystemMngr& sysMngr) {
	for (const auto& func : dynamicStartScheduleFuncs)
		func(schedule);

	for (const auto& func : staticStartScheduleFuncs)
		func(schedule);

	for (const auto& [n, func] : sysMngr.n2start)
		func(schedule);
}

void CmptSysMngr::GenSchedule(SystemSchedule<SysType::OnUpdate>& schedule, const SystemMngr& sysMngr) {
	for (const auto& func : dynamicUpdateScheduleFuncs)
		func(schedule);

	for (const auto& func : staticUpdateScheduleFuncs)
		func(schedule);

	for (const auto& [n, func] : sysMngr.n2update)
		func(schedule);
}

void CmptSysMngr::GenSchedule(SystemSchedule<SysType::OnStop>& schedule, const SystemMngr& sysMngr) {
	for (const auto& func : dynamicStopScheduleFuncs)
		func(schedule);

	for (const auto& func : staticStopScheduleFuncs)
		func(schedule);

	for (const auto& [n, func] : sysMngr.n2stop)
		func(schedule);
}
