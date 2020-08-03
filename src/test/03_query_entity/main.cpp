#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct A {};
struct B {};
struct C {};

class MySystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		schedule.Register(
			[em = schedule.GetEntityMngr()](Entity e, const A* a, const B* b) {
			em->AddCommand(
					[e, em]() {
						if (!em->Have(e, CmptType::Of<C>)) {
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
						if (em->Have(e, CmptType::Of<C>)) {
							cout << "Dettach C" << endl;
							em->Detach(e, &CmptType::Of<C>, 1);
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
