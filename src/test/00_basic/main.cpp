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

	for (size_t i = 0; i < 10; i++) {
		auto [entity, v, p] = w.CreateEntity<velocity, position>();
		entities.insert(entity);
		v->v = static_cast<float>(i);
	}

	for (size_t i = 0; i < 5; i++) {
		auto [entity, p] = w.CreateEntity<position>();
		auto [v] = entity->Attach<velocity>();
		entities.insert(entity);
		v->v = 10 + static_cast<float>(i);
	}

	for (size_t i = 0; i < 5; i++) {
		auto [entity, p] = w.CreateEntity<position>();
		entities.insert(entity);
	}
	
	for (auto e : entities)
		cout << e->Get<position>()->x << endl;

	int i = 0;
	auto sys = [&i](velocity* v, position* p) {
		p->x += v->v * 0.01f;
		cout << "i: " << i << endl;
		i++;
	};

	w.Each(sys);

	for (auto e : entities)
		cout << e->Get<position>()->x << endl;

	for (auto e : entities) {
		if (e->Get<velocity>()) {
			e->Detach<velocity>();
			e->Attach<velocity>();
		}
	}

	return 0;
}
