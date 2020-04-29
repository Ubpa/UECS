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

struct MoveSystem {
	static void OnSchedule(ScheduleRegistrar<SysType::OnUpdate>& schedule) {
		schedule.Register("Move", [](Position* p, const Velocity* v) {
			p->x += v->x;
			p->y += v->y;
		});
	}
};

int main() {
	CmptRegistrar::Instance().Register<Position, Velocity>();

	World w;
	w.Register<MoveSystem>();

	w.CreateEntity<Position, Velocity>();

	w.Start();
	w.Update();
	w.Stop();

	cout << w.DumpUpdateTaskflow() << endl;

	return 0;
}
