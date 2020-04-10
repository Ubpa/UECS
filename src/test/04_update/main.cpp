#include <UECS/World.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct Position {
	float x;
	void OnUpdate() const {
		cout << "position: " << x << endl;
	}
};

struct Velocity {
	float x;

	void Update(Position* p) const {
		p->x += x;
		cout << "update Position" << endl;
	}

	void Update() const {
		cout << "update Position(aha)" << endl;
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
		v->x += x;
		cout << "update Velocity" << endl;
	}
};

int main() {
	World w;
	w.CreateEntity<Velocity, Position, Acceleration>();
	w.Update(true);
	return 0;
}
