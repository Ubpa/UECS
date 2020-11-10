#include <UECS/World.h>

using namespace Ubpa::UECS;

struct Position { float val; };
struct Velocity { float val; };

struct MoverSystem {
	static void OnCreate(World* w) {
		std::cout << "OnCreate" << std::endl;
	}
	static void OnActivate(World* w) {
		std::cout << "OnActivate" << std::endl;
	}
	static void OnUpdate(Schedule& schedule) {
		std::cout << "OnUpdate" << std::endl;
		schedule.RegisterEntityJob(
			[](const Velocity* v, Position* p) {
				p->val += v->val;
			},
			"Mover"
		);
	}
	static void OnDeactivate(World* w) {
		std::cout << "OnDeactivate" << std::endl;
	}
	static void OnDestroy(World* w) {
		std::cout << "OnDestroy" << std::endl;
	}
};

int main() {
	World w;
	auto [move] = w.systemMngr.systemTraits.Register<MoverSystem>();
	w.entityMngr.Create<Position, Velocity>();
	w.systemMngr.Activate(move);
	w.Update();
}
