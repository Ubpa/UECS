#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct Data {};

class DataSystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		schedule.Register([](LastFrame<Data> d) { cout << "lastFrame_sys0" << endl; }, "lastFrame_sys0");
		schedule.Register([](LastFrame<Data> d) { cout << "lastFrame_sys1" << endl; }, "lastFrame_sys1");
		schedule.Register([](Data* d) { cout << "writer_sys0" << endl; }, "writer_sys0");
		schedule.Register([](Data* d) { cout << "writer_sys1" << endl; }, "writer_sys1");
		schedule.Register([](Data* d) { cout << "writer_sys2" << endl; }, "writer_sys2");
		schedule.Register([](const Data* d) { cout << "latest_sys0" << endl; }, "latest_sys0");
		schedule.Register([](const Data* d) { cout << "latest_sys1" << endl; }, "latest_sys1");
	}
};

int main() {
	World w;
	w.systemMngr.Register<DataSystem>();

	w.entityMngr.Create<Data>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
