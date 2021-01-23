#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace Ubpa;
using namespace std;

struct A {};
struct B {};

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		ArchetypeFilter filter_w0, filter_w1;
		filter_w0.none = { TypeID_of<A> };
		filter_w1.all = { AccessTypeID_of<A> };
		schedule.RegisterEntityJob([](B*) {}, "need B, none A", true, filter_w0);
		schedule.RegisterEntityJob([](B*) {}, "need A, B", true, filter_w1);;
	}
};

int main() {
	World w;
	w.systemMngr.RegisterAndActivate<MySystem>();
	w.entityMngr.Create<>();
	w.entityMngr.Create<A>();
	w.entityMngr.Create<B>();
	w.entityMngr.Create<A, B>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
