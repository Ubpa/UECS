#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;

struct Position { float val{ 0.f }; };
struct Velocity { float val{ 1.f }; };

class MoverSystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
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
	w.systemMngr.Register<MoverSystem>();
	w.entityMngr.Create<Position, Velocity>();

	auto copy_e = w.entityMngr;
	
	w.entityMngr.Swap(copy_e);
	w.Update();
	w.entityMngr.Create<Position, Velocity>();
	w.Update();
	std::cout << "swap" << std::endl;
	w.entityMngr.Swap(copy_e);
	w.Update();
}
