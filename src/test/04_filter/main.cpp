#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace Ubpa;
using namespace std;

struct A {};
struct B {};
struct C {};
struct D {};
struct E { float val; };

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		ArchetypeFilter filter;
		filter.all = { AccessTypeID_of<A> };
		filter.any = { AccessTypeID_of<B>, AccessTypeID_of<C> };
		filter.none = { TypeID_of<D> };

		schedule.RegisterEntityJob(
			[](const E* e) {
				cout << e->val << endl;
			},
			"test filter",
			true,
			filter
		);
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<A, B, C, D, E>();
	w.systemMngr.RegisterAndActivate<MySystem>();

	auto e0 = w.entityMngr.Create(Ubpa::TypeIDs_of<   B      , E>); // x
	auto e1 = w.entityMngr.Create(Ubpa::TypeIDs_of<A         , E>); // x
	auto e2 = w.entityMngr.Create(Ubpa::TypeIDs_of<A,    C, D, E>); // x
	auto e3 = w.entityMngr.Create(Ubpa::TypeIDs_of<A, B      , E>); // bingo

	w.entityMngr.Get<E>(e0)->val = 0.f;
	w.entityMngr.Get<E>(e1)->val = 1.f;
	w.entityMngr.Get<E>(e2)->val = 2.f;
	w.entityMngr.Get<E>(e3)->val = 3.f;

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
