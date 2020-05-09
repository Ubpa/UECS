#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct A {};
struct B {};
struct C {};

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.Request(
			[](Entity* e, const A* a, const B* b) {
				e->AddCommand(
					[e]() {
						if (!e->Get<C>()) {
							cout << "Attach C" << endl;
							e->Attach<C>();
						}
					}
				);
			}, "AB"
		);
		schedule.Request(
			[](Entity* e, const A* a, const B* b, const C* c) {
				e->AddCommand(
					[e]() {
						if (e->Get<C>()) {
							cout << "Dettach C" << endl;
							e->Detach<C>();
						}
					}
				);
			}, "ABC"
		);
	}
};

int main() {
	CmptRegistrar::Instance().Register<A, B, C>();

	World w;
	w.systemMngr.Register<MySystem>();

	w.CreateEntity<A, B>();

	w.Update();
	w.Update();
	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
