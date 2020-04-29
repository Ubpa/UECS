#include <UECS/World.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct MyCmpt {
	static void OnRegister() { cout << "OnRegister" << endl; }

	// before first update
	static void OnSchedule(ScheduleRegistrar<SysType::OnStart>& registrar) {
		cout << "OnSchedule OnStart" << endl;
	}
	void OnStart() { cout << "OnStart" << endl; }

	static void OnSchedule(ScheduleRegistrar<SysType::OnUpdate>& registrar) {
		cout << "OnSchedule OnUpdate" << endl;
	}
	void OnUpdate() { cout << "OnUpdate" << endl; }

	// after last update
	static void OnSchedule(ScheduleRegistrar<SysType::OnStop>& registrar) {
		cout << "OnSchedule OnStop" << endl;
	}
	void OnStop() { cout << "OnStop" << endl; }
};

int main() {
	CmptRegistrar::Instance().Register<MyCmpt>();

	World w;
	w.CreateEntity<MyCmpt>();

	w.Start();
	w.Update();
	w.Update();
	w.Stop();

	return 0;
}
