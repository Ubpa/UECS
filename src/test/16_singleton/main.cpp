#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;

struct Timer {
	float dt{ 0.f };
};

struct Position { float val{ 1.f }; };
struct Velocity { float val{ 1.f }; };

struct MoverSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterJob([](Singleton<Timer> timer) {
			timer->dt = 0.03f;
		}, "Timer");
		schedule.RegisterEntityJob([](const Velocity* v, Position* p, Latest<Singleton<Timer>> timer) {
			p->val += timer->dt * v->val;
		}, "Mover");
		schedule.RegisterEntityJob([](const Position* p) {
			std::cout << p->val << std::endl;
		}, "Print");
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<Timer, Position, Velocity>();
	w.systemMngr.RegisterAndActivate<MoverSystem>();

	w.entityMngr.Create(Ubpa::TypeIDs_of<Position, Velocity>);
	w.entityMngr.Create(Ubpa::TypeIDs_of<Timer>);

	w.Update();
	std::cout << w.DumpUpdateJobGraph() << std::endl;
	std::cout << w.GenUpdateFrameGraph().Dump() << std::endl;
}
