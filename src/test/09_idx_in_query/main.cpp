#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct A {};
struct B {};

class MySystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		auto flags = std::make_shared<std::vector<bool>>();
		auto f = schedule.Register(
			[flags](Entity e, size_t indexInQuery, const A*) {
				flags->at(indexInQuery) = true;
			}, "set flag"
		);
		schedule.Register(
			[flags]() {
				for (auto flag : *flags)
					cout << flag << endl;
			}, "print flag"
		);
		schedule.Order("set flag", "print flag");
		size_t num = GetWorld()->entityMngr.EntityNum(f->query);
		flags->insert(flags->begin(), num, false);
	}
};

int main() {
	World w;
	w.systemMngr.Register<MySystem>();

	w.entityMngr.Create<A>();
	w.entityMngr.Create<A>();
	w.entityMngr.Create<A, B>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
