#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct Data1 {};
struct Data2 {};

struct DataSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule
			.Register([](Data1* d1, Data2* d2) { cout << "writer_sys0" << endl; }, "writer_sys0")
			.Register([](Data1* d) { cout << "writer_sys1" << endl; }, "writer_sys1")
			.Register([](Data2* d2) { cout << "writer_sys2" << endl; }, "writer_sys2")
			.Register([](Data1* d, Data2* d2) { cout << "writer_sys3" << endl; }, "writer_sys3")
			.Order("writer_sys0", "writer_sys1")
			.Order("writer_sys1", "writer_sys3");
	}
};

int main() {
	World w;
	w.systemMngr.Register<DataSystem>();

	w.entityMngr.CreateEntity<Data1, Data2>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
