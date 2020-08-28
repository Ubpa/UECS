#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;

struct A {};
struct B {};
struct C {};

class PrintASystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		schedule.RegisterEntityJob([](A*) {
			std::cout << "A" << std::endl;
		}, "Parallel Print A");
		auto spilt = schedule.RegisterJob([]() {
			std::cout << "vvvvvvvvvv" << std::endl;
		}, "Spilt");
		schedule.RegisterEntityJob([](const A*) {
			std::cout << "A" << std::endl;
		}, "Serial Print A", false);
		schedule.Order("Parallel Print A", "Spilt");
		schedule.Order("Spilt", "Serial Print A");
	}
};

int main() {
	World w;
	w.systemMngr.Register<PrintASystem>();
	w.entityMngr.Create<A>();
	w.entityMngr.Create<A, B>();
	w.entityMngr.Create<A, C>();
	w.entityMngr.Create<A, B, C>();
	w.entityMngr.cmptTraits.Register
		<A, B, C>();

	for (size_t i = 0; i < 5; i++) {
		w.Update();
		std::cout << "^^^^^^^^^^" << std::endl;
	}

	for (size_t i = 0; i < 100; i++)
		w.entityMngr.Create();

	w.RunEntityJob([](Entity e) {
		std::cout << e.Idx() << std::endl;
	}, false);

	std::cout << w.DumpUpdateJobGraph() << std::endl;
	std::cout << w.GenUpdateFrameGraph().Dump() << std::endl;
}
