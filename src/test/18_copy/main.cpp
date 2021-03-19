#include <UECS/UECS.hpp>

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
	w.entityMngr.cmptTraits.Register<Position, Velocity>();
	w.systemMngr.RegisterAndActivate<MoverSystem>();
	w.entityMngr.Create(Ubpa::TypeIDs_of<Position, Velocity>);

	World cpW = w;
	
	cpW.Update();
	w.entityMngr.Create(Ubpa::TypeIDs_of<Position, Velocity>);
	cpW.Update();
}
