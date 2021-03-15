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
	w.systemMngr.RegisterAndActivate<MySystem>();

	w.entityMngr.cmptTraits
		.RegisterName(Type_of<A>)
		.RegisterName(Type_of<B>)
		.RegisterName(Type_of<C>)
		.RegisterName(Type_of<D>)
		.RegisterName(Type_of<E>)
		.RegisterName(Type_of<F>)
		.RegisterName(Type_of<G>)
		;

	w.entityMngr.Create<A, B, C, D, E>();
	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;
	cout << w.GenUpdateFrameGraph().Dump() << endl;

	return 0;
}
