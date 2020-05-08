#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct Data {};

struct DataSystem {
	static void OnUpdate(Schedule& schedule) {
		auto lastFrame_sys0 = schedule.Request(
			[](CmptTag::LastFrame<Data> d) {
				cout << "lastFrame_sys0" << endl;
			},
			"lastFrame_sys0");
		auto lastFrame_sys1 = schedule.Request(
			[](CmptTag::LastFrame<Data> d) {
				cout << "lastFrame_sys1" << endl;
			},
			"lastFrame_sys1");
		auto writer_sys0 = schedule.Request(
			[](Data* d) {
				cout << "writer_sys0" << endl;
			},
			"writer_sys0");
		auto writer_sys1 = schedule.Request(
			[](Data* d) {
				cout << "writer_sys1" << endl;
			},
			"writer_sys1");
		auto writer_sys2 = schedule.Request(
			[](Data* d) {
				cout << "writer_sys2" << endl;
			},
			"writer_sys2");
		auto writer_sys3 = schedule.Request(
			[](Data* d) {
				cout << "writer_sys3" << endl;
			},
			"writer_sys3");
		auto writer_sys4 = schedule.Request(
			[](Data* d) {
				cout << "writer_sys4" << endl;
			},
			"writer_sys4");
		auto writer_sys5 = schedule.Request(
			[](Data* d) {
				cout << "writer_sys5" << endl;
			},
			"writer_sys5");
		auto latest_sys0 = schedule.Request(
			[](const Data* d) {
				cout << "latest_sys0" << endl;
			},
			"latest_sys0");
		auto latest_sys1 = schedule.Request(
			[](const Data* d) {
				cout << "latest_sys1" << endl;
			},
			"latest_sys1");

		schedule
			.Order(writer_sys0, writer_sys1)
			.Order(writer_sys0, writer_sys2)
			.Order(writer_sys1, writer_sys3)
			.Order(writer_sys2, writer_sys3)
			.Order(writer_sys4, writer_sys5)
			;
	}
};

int main() {
	CmptRegistrar::Instance().Register<Data>();

	World w;
	w.systemMngr.Register<DataSystem>();

	w.CreateEntity<Data>();

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
