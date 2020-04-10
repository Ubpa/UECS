#include <UECS/World.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct Position {
	float x;
	void OnUpdate() const {
		//cout << "position: " << x << endl;
	}
};

struct Velocity {
	float x;

	void Update(Position* p) const {
		for(size_t i=0;i< 1000000;i++)
			p->x *= x;
	}

	void Update() const {
	}

	static void OnSchedule(std::map<SystemMngr::ScheduleType, SystemSchedule*>& type2schedule) {
		(*type2schedule[SystemMngr::ScheduleType::OnUpdate])
			.Regist(MemFuncOf<void(Position*)const>::run(&Velocity::Update))
			.Regist(MemFuncOf<void()const>::run(&Velocity::Update));
	}
};

struct Acceleration {
	float x;

	void OnUpdate(Velocity* v) const {
		for (size_t i = 0; i < 1000000; i++)
			v->x *= x;
	}
};

int main() {
	World w;
	for (size_t i = 0; i < 10000; i++)
		w.CreateEntity<Velocity, Position, Acceleration>();
	w.Update(true);
	return 0;
}
