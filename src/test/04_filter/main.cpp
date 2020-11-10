#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace Ubpa;
using namespace std;

struct A {};
struct B {};
struct C {};
struct D {};
struct E { float val; };

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		ArchetypeFilter filter;
		filter.all = { CmptAccessType::Of<A> };
		filter.any = { CmptAccessType::Of<B>, CmptAccessType::Of<C> };
		filter.none = { CmptType::Of<D> };

		schedule.RegisterEntityJob(
			[](const E* e) {
				cout << e->val << endl;
			},
			"test filter",
			true,
			filter
		);
	}
};

int main() {
	World w;
	auto [mySystem] = w.systemMngr.systemTraits.Register<MySystem>();

	auto [e0,     b0        ] = w.entityMngr.Create<   B      >(); // x
	auto [e1, a1            ] = w.entityMngr.Create<A         >(); // x
	auto [e2, a2,     c2, d2] = w.entityMngr.Create<A,    C, D>(); // x
	auto [e3, a3, b3        ] = w.entityMngr.Create<A, B      >(); // bingo

	w.entityMngr.Emplace<E>(e0, 0.f);
	w.entityMngr.Emplace<E>(e1, 1.f);
	w.entityMngr.Emplace<E>(e2, 2.f);
	w.entityMngr.Emplace<E>(e3, 3.f);

	w.systemMngr.Activate(mySystem);
	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
