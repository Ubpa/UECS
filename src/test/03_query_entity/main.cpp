#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct A {};
struct B {};
struct C {};

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.Register(
			[em = schedule.GetEntityMngr()](Entity e, const A* a, const B* b) {
			em->AddCommand(
					[e, em]() {
						if (!em->Have<C>(e)) {
							cout << "Attach C" << endl;
							em->Attach<C>(e);
						}
					}
				);
			}, "AB"
		);
		schedule.Register(
			[em = schedule.GetEntityMngr()](Entity e, const A* a, const B* b, const C* c) {
				em->AddCommand(
					[e, em]() {
						if (em->Have<C>(e)) {
							cout << "Dettach C" << endl;
							em->Detach<C>(e);
						}
					}
				);
			}, "ABC"
		);
	}
};

int main() {
	World w;
	w.systemMngr.Register<MySystem>();

	w.entityMngr.Create<A, B>();

	w.Update();
	w.Update();
	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
