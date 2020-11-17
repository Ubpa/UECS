#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

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

void print0(const World& w) {
	w.RunEntityJob([](const World* w, const Velocity* v, const Position* p) {
		cout << v->val << ", " << p->val << endl;
	}, false);
}

void print1(const World& w) {
	ArchetypeFilter filter;
	filter.all = { CmptAccessType::Of<Latest<Velocity>>, CmptAccessType::Of<Latest<Position>> };
	w.RunChunkJob(
		[](const World* w, ChunkView chunk) {
			auto velocities = chunk.GetCmptArray<Velocity>();
			auto positions = chunk.GetCmptArray<Position>();
			size_t N = chunk.EntityNum();
			for (size_t i = 0; i < N; i++) {
				cout << velocities[i].val << ", " << positions[i].val << endl;
			}
		},
		filter,
		false
	);
}

int main() {
	World w;
	w.systemMngr.RegisterAndActivate<MoverSystem>();
	w.entityMngr.Create<Position, Velocity>();
	w.Update();
	print0(w);
	print1(w);
}
