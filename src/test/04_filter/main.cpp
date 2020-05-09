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
		schedule.Request(
			[](const E* e) {
				cout << e->val << endl;
			}, "test filter", filter
		);
	}
};

int main() {
	CmptRegistrar::Instance().Register<A, B, C, D, E>();

	World w;
	w.systemMngr.Register<MySystem>();

	auto [e0,     b0        ] = w.CreateEntity<   B      >(); // x
	auto [e1, a1            ] = w.CreateEntity<A         >(); // x
	auto [e2, a2,     c2, d2] = w.CreateEntity<A,    C, D>(); // x
	auto [e3, a3, b3        ] = w.CreateEntity<A, B      >(); // bingo

	e0->AssignAttach<E>(0.f);
	e1->AssignAttach<E>(1.f);
	e2->AssignAttach<E>(2.f);
	e3->AssignAttach<E>(3.f);

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
