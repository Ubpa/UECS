#include <UECS/World.h>

using namespace Ubpa::UECS;

struct Position { float val; };
struct Velocity { float val; };

class MoverSystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		schedule
			.RegisterEntityJob(
				[](const Velocity* v, Position* p) {
					p->val += v->val;
				},
				"Mover"
			);
	}
};

int main() {
	World w;
	w.systemMngr.Register<MoverSystem>();
	w.entityMngr.Create<Position, Velocity>();
	w.Update();
}
