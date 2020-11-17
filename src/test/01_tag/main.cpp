#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct Data {};

struct DataSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob([](LastFrame<Data> d) { cout << "lastFrame_sys0" << endl; }, "lastFrame_sys0");
		schedule.RegisterEntityJob([](LastFrame<Data> d) { cout << "lastFrame_sys1" << endl; }, "lastFrame_sys1");
		schedule.RegisterEntityJob([](Data* d) { cout << "writer_sys0" << endl; }, "writer_sys0");
		schedule.RegisterEntityJob([](Data* d) { cout << "writer_sys1" << endl; }, "writer_sys1");
		schedule.RegisterEntityJob([](Data* d) { cout << "writer_sys2" << endl; }, "writer_sys2");
		schedule.RegisterEntityJob([](const Data* d) { cout << "latest_sys0" << endl; }, "latest_sys0");
		schedule.RegisterEntityJob([](const Data* d) { cout << "latest_sys1" << endl; }, "latest_sys1");
	}
};

int main() {
	World w;
	w.systemMngr.RegisterAndActivate<DataSystem>();

	w.entityMngr.Create<Data>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
