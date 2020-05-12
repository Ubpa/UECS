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

	for (size_t i = 0; i < 10; i++)
		w.entityMngr.CreateEntity<>();

	w.Update();
}
