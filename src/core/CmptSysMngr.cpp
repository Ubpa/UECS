#include <UECS/detail/CmptSysMngr.h>

using namespace Ubpa;

void CmptSysMngr::GenSchedule(ScheduleRegistrar<SysType::OnStart>& registrar, const SystemMngr& sysMngr) {
	for (const auto& func : dynamicStartScheduleFuncs)
		func(registrar);

	for (const auto& func : staticStartScheduleFuncs)
		func(registrar);

	for (const auto& [n, func] : sysMngr.n2start)
		func(registrar);
}

void CmptSysMngr::GenSchedule(ScheduleRegistrar<SysType::OnUpdate>& registrar, const SystemMngr& sysMngr) {
	for (const auto& func : dynamicUpdateScheduleFuncs)
		func(registrar);

	for (const auto& func : staticUpdateScheduleFuncs)
		func(registrar);

	for (const auto& [n, func] : sysMngr.n2update)
		func(registrar);
}

void CmptSysMngr::GenSchedule(ScheduleRegistrar<SysType::OnStop>& registrar, const SystemMngr& sysMngr) {
	for (const auto& func : dynamicStopScheduleFuncs)
		func(registrar);

	for (const auto& func : staticStopScheduleFuncs)
		func(registrar);

	for (const auto& [n, func] : sysMngr.n2stop)
		func(registrar);
}
