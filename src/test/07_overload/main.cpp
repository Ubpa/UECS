#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct P {};
struct V {};
struct A {};

struct VP_System {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob([](const V*, P*) {cout << "VP" << endl; }, "VP");
	}
};

struct AVP_System {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob([](const A*, V*, P*) {cout << "AVP" << endl; }, "AVP");
		schedule.InsertNone("VP", CmptType::Of<A>);
	}
};

int main() {
	World w;
	w.systemMngr.RegisterAndActivate<VP_System, AVP_System>();

	w.entityMngr.Create<V, P>();
	w.entityMngr.Create<A, V, P>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
