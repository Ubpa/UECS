#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;

struct Position { float val{ 0.f }; };
struct Velocity { float val{ 1.f }; };

struct MoverSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](const Velocity* v, Position* p) {
				p->val += v->val;
			},
			"Mover"
		);
		schedule.RegisterEntityJob(
			[](const Velocity* v, const Position* p) {
				std::cout << "v:" << v->val << std::endl;
				std::cout << "p:" << p->val << std::endl;
			},
			"Print"
		);
	}
};

int main() {
	World w;
	w.systemMngr.RegisterAndActivate<MoverSystem>();
	w.entityMngr.Create<Position, Velocity>();

	World cpW = w;
	
	cpW.Update();
	cpW.entityMngr.Create<Position, Velocity>();
	cpW.Update();
}
