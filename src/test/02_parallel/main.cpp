#include <UECS/core/World.h>

#include <iostream>
#include <random>
#include <chrono>

using namespace std;
using namespace Ubpa;

struct velocity { float v{ 0.f }; };
struct position { float x{ 0.f }; };

int main() {
	constexpr size_t N = 1000000;
	World w;
	set<Entity*> entities;

	default_random_engine engine;
	uniform_real_distribution uniform(0.f, 1.f);

	for (size_t i = 0; i < N; i++) {
		auto [e,v,p] = w.CreateEntity<velocity, position>();
		v->v = uniform(engine);
		p->x = uniform(engine);
		entities.insert(e);
	}

	auto sys = [](velocity* v, position* x) {
		for (size_t i = 0; i < 6000; i++)
			x->x += v->v * 0.01f;
	};

	auto t0 = chrono::steady_clock::now();
	w.Each(sys);
	auto t1 = chrono::steady_clock::now();
	w.ParallelEach(sys);
	auto t2 = chrono::steady_clock::now();

	chrono::duration<double> t01 = chrono::duration_cast<chrono::duration<double>>(t1 - t0);
	chrono::duration<double> t12 = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
	cout << "Each use: " << t01.count() << "s" << endl;
	cout << "core: " << thread::hardware_concurrency() << endl;
	cout << "ParallelEach use: " << t12.count() << "s" << endl;

	for (auto e : entities)
		cout << e->Get<position>()->x << endl;

	return 0;
}
