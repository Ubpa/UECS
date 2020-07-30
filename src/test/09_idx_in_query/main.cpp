#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct A {};
struct B {};

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		auto flags = std::make_shared<std::vector<bool>>();
		schedule
			.Register(
				[flags](Entity e, size_t indexInQuery, const A*) {
					flags->at(indexInQuery) = true;
				}, "set flag"
			)
			.Register(
				[flags]() {
					for (auto flag : *flags)
						cout << flag << endl;
				}, "print flag"
			)
			.Order("set flag", "print flag");
		size_t num = schedule.EntityNumInQuery("set flag");
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
