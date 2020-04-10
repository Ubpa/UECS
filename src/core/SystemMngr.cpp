#include <UECS/detail/SystemMngr.h>

using namespace Ubpa;
using namespace std;

SystemMngr::SystemMngr(ArchetypeMngr* archetypeMngr)
{
	type2schedule[ScheduleType::OnUpdate] = new SystemSchedule{ archetypeMngr };
}

SystemMngr::~SystemMngr() {
	for (const auto& [type, schedule] : type2schedule)
		delete schedule;
}

void SystemMngr::GenTaskflow(std::map<ScheduleType, tf::Taskflow>& type2tf) {
	type2tf.clear();

	for (const auto& [type, schedule] : type2schedule)
		schedule->Clear();

	for (auto& func : dynamicScheduleFuncs)
		func(type2schedule);

	for (const auto& [type, staticScheduleFuncs] : type2StaticScheduleFuncs) {
		for(const auto& func : staticScheduleFuncs)
			func(type2schedule[type]);
	}

	for (const auto& [type, schedule] : type2schedule)
		schedule->GenTaskflow(type2tf[type]);
}
