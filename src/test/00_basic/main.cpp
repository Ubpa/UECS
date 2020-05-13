#include <UECS/World.h>

struct Position { float val; };
struct Velocity { float val; };

struct MoverSystem {
	static void OnUpdate(Ubpa::Schedule& schedule) {
		schedule.Request(
			[](const Velocity* v, Position* p) {
				p->val += v->val;
			}, "MoverSystem");
	}
};

int main() {
	Ubpa::World w;
	w.systemMngr.Register<MoverSystem>();
	w.entityMngr.CreateEntity<Position, Velocity>();
	w.Update();
}
