#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct A {};
struct B {};
struct C {};

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](World* w, Entity e, const A* a, const B* b) {
				w->AddCommand(
					[e, w]() {
						if (!w->entityMngr.Have(e, CmptType::Of<C>)) {
							cout << "Attach C" << endl;
							w->entityMngr.Attach<C>(e);
						}
					}
				);
			},
			"AB"
		);
		schedule.RegisterEntityJob(
			[](World* w, Entity e, const A* a, const B* b, const C* c) {
				w->AddCommand(
					[e, w]() {
						if (w->entityMngr.Have(e, CmptType::Of<C>)) {
							cout << "Dettach C" << endl;
							w->entityMngr.Detach(e, &CmptType::Of<C>, 1);
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
	auto mySystem = w.systemMngr.Register<MySystem>();

	w.entityMngr.Create<A, B>();

	w.systemMngr.Activate(mySystem);
	w.Update();
	w.Update();
	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
