#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct Data1 {};
struct Data2 {};

class DataSystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) noexcept {
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

	w.entityMngr.Create<Data1, Data2>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
