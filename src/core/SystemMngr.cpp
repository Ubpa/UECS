#include <UECS/detail/SystemMngr.h>

using namespace Ubpa;
using namespace std;

void SystemMngr::GenStartSchedule(SystemSchedule& schedule) {
	for (const auto& func : dynamicStartScheduleFuncs)
		func(schedule);

	for (const auto& func : staticStartScheduleFuncs)
		func(schedule);
}

void SystemMngr::GenUpdateSchedule(SystemSchedule& schedule) {
	for (const auto& func : dynamicUpdateScheduleFuncs)
		func(schedule);

	for (const auto& func : staticUpdateScheduleFuncs)
		func(schedule);
}

void SystemMngr::GenStopSchedule(SystemSchedule& schedule) {
	for (const auto& func : dynamicStopScheduleFuncs)
		func(schedule);

	for (const auto& func : staticStopScheduleFuncs)
		func(schedule);
}
