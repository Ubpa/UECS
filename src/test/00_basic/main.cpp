#include <UECS/World.h>

struct Position { float val; };
struct Velocity { float val; };

struct MoverSystem {
	static void OnUpdate(Ubpa::Schedule& schedule) {
		schedule.Register(
			[](const Velocity* v, Position* p) {
				p->val += v->val;
			}, "Mover");
	}
};

int main() {
	Ubpa::World w;
	w.systemMngr.Register<MoverSystem>();
	w.entityMngr.Create<Position, Velocity>();
	w.Update();
}
