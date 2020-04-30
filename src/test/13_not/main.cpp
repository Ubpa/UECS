#include <UECS/World.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct Position {
	float x, y;
};

struct Velocity {
	float x, y;
};

struct Acceleration {
	float x, y;
};

struct MoveSystem_VP {
	static void OnSchedule(ScheduleRegistrar<SysType::OnUpdate>& schedule) {
		schedule.Register("MoveSystem_VP", [](CmptTag::None<Acceleration>, Position* p, const Velocity* v) {
			p->x += v->x;
			p->y += v->y;
			cout << "MoveSystem_VP" << endl;
		});
	}
};

struct MoveSystem_AVP {
	static void OnSchedule(ScheduleRegistrar<SysType::OnUpdate>& schedule) {
		schedule.Register("MoveSystem_AVP", [](Position* p, Velocity* v, const Acceleration* a) {
			p->x += v->x + a->x / 2;
			p->y += v->y + a->y / 2;
			v->x += a->x;
			v->y += a->y;
			cout << "MoveSystem_AVP" << endl;
		});
	}
};

int main() {
	CmptRegistrar::Instance().Register<Position, Velocity, Acceleration>();

	World w;
	w.Register<MoveSystem_VP, MoveSystem_AVP>();

	w.CreateEntity<Position, Velocity>();
	w.CreateEntity<Position, Velocity>();
	w.CreateEntity<Position, Velocity, Acceleration>();

	w.Start();
	w.Update();
	w.Stop();

	cout << w.DumpUpdateTaskflow() << endl;

	return 0;
}
