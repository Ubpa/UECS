#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct P {};
struct V {};
struct A {};

class VP_System : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		schedule.Register([](const V*, P*) {cout << "VP" << endl; }, "VP");
	}
};

class AVP_System : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		schedule
			.Register([](const A*, V*, P*) {cout << "AVP" << endl; }, "AVP")
			.InsertNone("VP", CmptType::Of<A>);
	}
};

int main() {
	World w;
	w.systemMngr.Register<VP_System, AVP_System>();

	w.entityMngr.Create<V, P>();
	w.entityMngr.Create<A, V, P>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
