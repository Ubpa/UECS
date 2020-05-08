#include <UECS/World.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct Position {
	float x;
	float y;
};

struct Velocity {
	float x;
	float y;
};

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		auto vp_sys = schedule.Request(
			[](const Velocity* v, Position* p) {
				p->x += v->x;
				p->y += v->y;
			},
			"VP System"
		);
	}
};

int main() {
	CmptRegistrar::Instance().Register<Position, Velocity>();

	World w;
	w.systemMngr.Register<MySystem>();

	for (size_t i = 0; i < 10; i++)
		w.CreateEntity<Position, Velocity>();

	w.Update();
	
	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
