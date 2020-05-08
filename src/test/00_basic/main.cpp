#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct Position {
	Position(float f) :val{ f } {}
	float val;
};
struct Velocity {
	Velocity(float f) :val{ f } {}
	float val;
};

struct MoverSystem {
	static void OnUpdate(Schedule& schedule) {
		auto vp_sys = schedule.Request(
			[](const Velocity* v, Position* p) {
				p->val += v->val;
				cout << p->val << endl;
			},
			"MoverSystem");
	}
};

int main() {
	std::is_constructible_v<Position, float>;

	CmptRegistrar::Instance().Register<Position, Velocity>();

	World w;
	w.systemMngr.Register<MoverSystem>();

	for (size_t i = 0; i < 10; i++) {
		auto [e] = w.CreateEntity<>();
		float fi = static_cast<float>(i);
		e->AssignAttach<Position>(fi);
		e->AssignAttach<Velocity>(2 * fi);
	}

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
