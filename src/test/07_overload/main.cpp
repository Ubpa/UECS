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
	auto vpSystem = w.systemMngr.Register<VP_System>();
	auto avpSystem = w.systemMngr.Register<AVP_System>();

	w.entityMngr.Create<V, P>();
	w.entityMngr.Create<A, V, P>();

	w.systemMngr.Activate(vpSystem);
	w.systemMngr.Activate(avpSystem);
	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
