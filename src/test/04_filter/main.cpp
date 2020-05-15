#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct A {};
struct B {};
struct C {};
struct D {};
struct E {
	E(float f) :val{ f } {}
	float val;
};

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		EntityFilter filter(TypeList<A>{}, // all
			TypeList<B, C>{}, // any
			TypeList<D>{} // none
		);
		schedule.Register(
			[](const E* e) {
				cout << e->val << endl;
			}, "test filter", filter
		);
	}
};

int main() {
	World w;
	w.systemMngr.Register<MySystem>();

	auto [e0,     b0        ] = w.entityMngr.CreateEntity<   B      >(); // x
	auto [e1, a1            ] = w.entityMngr.CreateEntity<A         >(); // x
	auto [e2, a2,     c2, d2] = w.entityMngr.CreateEntity<A,    C, D>(); // x
	auto [e3, a3, b3        ] = w.entityMngr.CreateEntity<A, B      >(); // bingo

	w.entityMngr.Emplace<E>(e0, 0.f);
	w.entityMngr.Emplace<E>(e1, 1.f);
	w.entityMngr.Emplace<E>(e2, 2.f);
	w.entityMngr.Emplace<E>(e3, 3.f);

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
