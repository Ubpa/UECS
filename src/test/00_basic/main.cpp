#include <UECS/World.h>

using namespace Ubpa;
using namespace Ubpa::UECS;

struct Position { float val; };
struct Velocity { float val; };

struct MoverSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](const Velocity* v, Position* p) {
				p->val += v->val;
			},
			"Mover"
		);
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<Position, Velocity>();
	w.systemMngr.RegisterAndActivate<MoverSystem>();
	w.entityMngr.Create(TypeIDs_of<Position, Velocity>);
	w.Update();
}
