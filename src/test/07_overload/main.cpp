#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct P {};
struct V {};
struct A {};

struct VP_System {
	static void OnUpdate(Schedule& schedule) {
		schedule.Register([](const V*, P*) {cout << "VP" << endl; }, "VP");
	}
};

struct AVP_System {
	static void OnUpdate(Schedule& schedule) {
		schedule
			.Register([](const A*, V*, P*) {cout << "AVP" << endl; }, "AVP")
			.InsertNone<A>("VP");
	}
};

int main() {
	World w;
	w.systemMngr.Register<VP_System, AVP_System>();

	w.entityMngr.CreateEntity<V, P>();
	w.entityMngr.CreateEntity<A, V, P>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
