#include <UECS/core/World.h>

#include <iostream>
#include <set>

using namespace std;
using namespace Ubpa;

struct velocity {
	velocity() = default;
	velocity(float v) :v(v) {};
	float v{ 0.f };
};

struct position {
	float x{ 0.f };
};

int main() {
	World w;
	set<Entity*> entities;

	auto e = w.CreateEntity<>();
	e->Attach<velocity>();
	e->Detach<velocity>();
	for (size_t i = 0; i < 10; i++)
		w.CreateEntity<>();

	// [ invalid ]
	//size_t i = 0;
	//w.Each([&i]() {
	//	cout << "i: " << i << endl;
	//	i++;
	//	});
}
