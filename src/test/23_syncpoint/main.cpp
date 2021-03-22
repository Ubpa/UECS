#include <UECS/UECS.hpp>

using namespace Ubpa;
using namespace Ubpa::UECS;

struct Position { float val; };
struct Velocity { float val; };

struct MoverSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](const Velocity* v, Position* p) {
				std::cout << "Pre Mover" << std::endl;
				p->val += v->val;
			},
			"Pre Mover",
			Schedule::EntityJobConfig {
				.layer = -1
			}
		);
		schedule.RegisterEntityJob(
			[](World* w, Entity e, CommandBufferView cbv) {
				std::cout << "Attach Velocity" << std::endl;
				cbv->AddCommand([w, e]() {
					w->entityMngr.Attach(e, TypeIDs_of<Velocity>);
					w->entityMngr.WriteComponent<Velocity>(e)->val = 1.f;
				});
			},
			"Attach Velocity",
			Schedule::EntityJobConfig {
				.archetypeFilter = {
					.all = { AccessTypeID_of<Latest<Position>> },
					.none = { TypeID_of<Velocity> },
				},
				.layer = 0
			}
		);
		schedule.RegisterEntityJob(
			[](const Velocity* v, Position* p) {
				std::cout << "Post Mover" << std::endl;
				p->val += v->val;
			},
			"Post Mover",
			Schedule::EntityJobConfig {
				.layer = 1
			}
		);
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<Position, Velocity>();
	w.systemMngr.RegisterAndActivate<MoverSystem>();
	w.entityMngr.Create(TypeIDs_of<Position>);
	w.Update();
}
