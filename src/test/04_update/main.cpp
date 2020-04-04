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

	void OnUpdate(Position* p) const {
		p->x += x;
		cout << "update Position" << endl;
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
	auto [e0, v0, p0, a0] = w.CreateEntity<Velocity, Position, Acceleration>();
	w.Update(true);
	return 0;
}
