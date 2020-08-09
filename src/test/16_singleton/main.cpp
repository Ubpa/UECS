#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;

struct Timer {
	float dt{ 0.f };
};

struct Position { float val{ 1.f }; };
struct Velocity { float val{ 1.f }; };

class MoverSystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		schedule.Register([](Singleton<Timer> timer) {
			timer->dt = 0.03f;
		}, "Timer");
		schedule.Register([](const Velocity* v, Position* p, Latest<Singleton<Timer>> timer) {
			p->val += timer->dt * v->val;
		}, "Mover");
		schedule.Register([](const Position* p) {
			std::cout << p->val << std::endl;
		}, "Print");
	}
};

int main() {
	RTDCmptTraits::Instance().Register
		<Timer, Velocity, Position>();

	World w;
	w.systemMngr.Register<MoverSystem>();
	w.entityMngr.Create<Position, Velocity>();
	w.entityMngr.Create<Timer>();

	w.Update();
	std::cout << w.DumpUpdateJobGraph() << std::endl;
	std::cout << w.GenUpdateFrameGraph().Dump() << std::endl;
}
