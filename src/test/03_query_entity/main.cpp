#include <UECS/UECS.hpp>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UECS;
using namespace std;

struct A {};
struct B {};
struct C {};

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](World* w, Entity e, const A* a, const B* b, CommandBufferView cbv) {
				cbv->AddCommand(
					[e, w]() {
						if (!w->entityMngr.Have(e, TypeID_of<C>)) {
							cout << "Attach C" << endl;
							w->entityMngr.Attach(e, TypeIDs_of<C>);
						}
					}
				);
			},
			"AB"
		);
		schedule.RegisterEntityJob(
			[](World* w, Entity e, const A* a, const B* b, const C* c, CommandBufferView cbv) {
				cbv->AddCommand(
					[e, w]() {
						if (w->entityMngr.Have(e, TypeID_of<C>)) {
							cout << "Dettach C" << endl;
							w->entityMngr.Detach(e, TypeIDs_of<C>);
						}
					}
				);
			},
			"ABC"
		);
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<A, B, C>();
	w.systemMngr.RegisterAndActivate<MySystem>();

	w.entityMngr.Create(Ubpa::TypeIDs_of<A, B>);

	w.Update();
	w.Update();
	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
