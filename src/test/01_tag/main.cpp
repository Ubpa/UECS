#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct Data {};

struct DataSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule
			.Register([](CmptTag::LastFrame<Data> d) { cout << "lastFrame_sys0" << endl; }, "lastFrame_sys0")
			.Register([](CmptTag::LastFrame<Data> d) { cout << "lastFrame_sys1" << endl; }, "lastFrame_sys1")
			.Register([](Data* d) { cout << "writer_sys0" << endl; }, "writer_sys0")
			.Register([](Data* d) { cout << "writer_sys1" << endl; }, "writer_sys1")
			.Register([](Data* d) { cout << "writer_sys2" << endl; }, "writer_sys2")
			.Register([](const Data* d) { cout << "latest_sys0" << endl; }, "latest_sys0")
			.Register([](const Data* d) { cout << "latest_sys1" << endl; }, "latest_sys1");
	}
};

int main() {
	World w;
	w.systemMngr.Register<DataSystem>();

	w.entityMngr.CreateEntity<Data>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
