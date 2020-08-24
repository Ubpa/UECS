#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace Ubpa;
using namespace std;

struct A {};
struct B {};

class MySystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		ArchetypeFilter filter_w0, filter_w1;
		filter_w0.none = { CmptType::Of<A> };
		filter_w1.all = { CmptAccessType::Of<A> };
		schedule.RegisterEntityJob([](B*) {}, "need B, none A", filter_w0);
		schedule.RegisterEntityJob([](B*) {}, "need A, B", filter_w1);;
	}
};

int main() {
	World w;
	w.systemMngr.Register<MySystem>();
	w.entityMngr.Create<>();
	w.entityMngr.Create<A>();
	w.entityMngr.Create<B>();
	w.entityMngr.Create<A, B>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
