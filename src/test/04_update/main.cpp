#include <UECS/World.h>
#include <UECS/SystemTraits.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct alignas(8) Position {
	float x;
	void OnUpdate() const {
		//cout << "position: " << x << endl;
	}
};

struct alignas(8) EntityHandle {
	Entity* e;
};

struct alignas(8) Velocity {
	float x;

	void Update(Position* p) const {
		for(size_t i=0;i< 1000000;i++)
			p->x *= x;
	}

	void Update() const {
	}

	static void OnSchedule(SystemSchedule<SysType::OnUpdate>& schedule) {
		schedule
			.Regist(MemFuncOf<void(Position*)const>::run(&Velocity::Update))
			.Regist(MemFuncOf<void()const>::run(&Velocity::Update));
	}
};

struct alignas(8) Acceleration {
	float x;

	void OnUpdate(Velocity* v) const {
		for (size_t i = 0; i < 1000000; i++)
			v->x *= x;
	}
};

int main() {
	Require<HaveOnStartSchedule, Velocity>;
	CmptRegister::Instance().Regist<Position, Velocity, Acceleration, EntityHandle>();

	World w;
	for (size_t i = 0; i < 2; i++) {
		auto [e, v, p, eh] = w.CreateEntity<Velocity, Position, EntityHandle>();
		eh->e = e;
	}
	w.Update();
	cout << w.DumpUpdateTaskflow() << endl;
	w.Each([&w](EntityHandle* eh, Velocity*) {
		w.AddCommand([e = eh->e](){
			e->Attach<Acceleration>();
		});
	});
	w.Update();
	cout << w.DumpUpdateTaskflow() << endl;
	return 0;
}
