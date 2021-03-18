#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace Ubpa;
using namespace std;

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};
struct F {};
struct G {};

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		ArchetypeFilter filter;
		filter.all = { AccessTypeID_of<D> };
		filter.any = { AccessTypeID_of<E>, AccessTypeID_of<F> };
		filter.none = { TypeID_of<G> };
		schedule.RegisterEntityJob(
			[](LastFrame<A> a, Write<B> b, Latest<C> c) {},
			"System Func",
			true,
			filter
		);
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<A, B, C, D, E, F, G>();
	w.systemMngr.RegisterAndActivate<MySystem>();

	w.entityMngr.Create(Ubpa::TypeIDs_of<A, B, C, D, E>);
	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;
	cout << w.GenUpdateFrameGraph().Dump() << endl;

	return 0;
}
