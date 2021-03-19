#include <UECS/UECS.hpp>

#include <iostream>

using namespace Ubpa;
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
		schedule.AddNone("VP", TypeID_of<A>);
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<A, V, P>();
	w.systemMngr.RegisterAndActivate<VP_System, AVP_System>();

	w.entityMngr.Create(Ubpa::TypeIDs_of<   V, P>);
	w.entityMngr.Create(Ubpa::TypeIDs_of<A, V, P>);

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
