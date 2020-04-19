#include <UECS/World.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct Position {
	float x;
	static void OnRegist() {
		cout << "regist Position" << endl;
	}

	void OnUpdate() const {
		cout << "position: " << x << endl;
	}
};

struct Velocity {
	float x;

	static void OnRegist() {
		cout << "regist Velocity" << endl;
	}

	void OnUpdate(Position* p) const {
		cout << "velocity: " << x << endl;
		p->x += x;
	}
};

struct Acceleration {
	float x;

	void OnUpdate(Velocity* v) const {
		cout << "Acceleration: " << x << endl;
		v->x += x;
	}
};

int main() {
	World w;
	w.CreateEntity<Velocity, Position, Acceleration>();
	w.Update();
	cout << w.DumpUpdateTaskflow() << endl;
	return 0;
}
