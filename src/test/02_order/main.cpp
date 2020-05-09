#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct Data1 {};
struct Data2 {};

struct DataSystem {
	static void OnUpdate(Schedule& schedule) {
		auto s0 = schedule.Request([](Data1* d1, Data2* d2) { cout << "writer_sys0" << endl; }, "writer_sys0");
		auto s1 = schedule.Request([](Data1* d) { cout << "writer_sys1" << endl; }, "writer_sys1");
		auto s2 = schedule.Request([](Data2* d2) { cout << "writer_sys2" << endl; }, "writer_sys2");
		auto s3 = schedule.Request([](Data1* d, Data2* d2) { cout << "writer_sys3" << endl; }, "writer_sys3");

		schedule.Order(s0, s1)
			.Order(s1, s3);
	}
};

int main() {
	CmptRegistrar::Instance().Register<Data1, Data2>();

	World w;
	w.systemMngr.Register<DataSystem>();

	w.CreateEntity<Data1, Data2>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
