#include <UECS/core/World.h>

#include <iostream>
#include <random>
#include <chrono>
#include <random>

using namespace std;
using namespace Ubpa;

struct velocity { float v{ 0.f }; float pad[31]{}; };
struct position { float x{ 0.f }; float pad[31]{}; };

int main() {
	constexpr size_t N = 1<<22;
	constexpr size_t M = 100;

	World w;
	vector<velocity*> vs;
	vector<position*> ps;
	vector<pair<size_t, size_t>> table;
	vs.reserve(N);
	ps.reserve(N);
	table.reserve(N);

	default_random_engine engine;
	uniform_real_distribution uni_f(0.f, 1.f);
	uniform_int_distribution uni_i;

	for (size_t i = 0; i < N; i++) {
		auto [e,v,p] = w.CreateEntity<velocity, position>();
		v->v = uni_f(engine);
		p->x = uni_f(engine);
		vs.push_back(v);
		ps.push_back(p);
	}

	//reorder
	for (size_t i = 0; i < N; i++) {
		auto vidx = uni_i(engine) % (N - i);
		auto pidx = uni_i(engine) % (N - i);
		vs[i] = vs[vidx + i];
		ps[i] = ps[pidx + i];
		table.push_back({ vidx, pidx });
	}

	auto sys = [M](velocity* v, position* x) {
		for (size_t i = 0; i < M; i++)
			x->x += v->v * 0.01f;
	};

	auto t0 = chrono::steady_clock::now();
	for (size_t i = 0; i < N; i++) {
		// simulate entity pointer
		auto vpidx = table[i];
		auto p = ps[vpidx.first];
		auto v = ps[vpidx.second];
		// component pointer
		auto& pX = p->x;
		auto vX = v->x;
		for (size_t i = 0; i < M; i++)
			pX += vX * 0.01f;
	}

	auto t1 = chrono::steady_clock::now();
	w.Each(sys);
	auto t2 = chrono::steady_clock::now();
	w.ParallelEach(sys);
	auto t3 = chrono::steady_clock::now();

	chrono::duration<double> t01 = chrono::duration_cast<chrono::duration<double>>(t1 - t0);
	chrono::duration<double> t12 = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
	chrono::duration<double> t32 = chrono::duration_cast<chrono::duration<double>>(t3 - t2);

	cout << "core: " << thread::hardware_concurrency() << endl;
	cout << "[   Native   ] consume: " << t01.count() << "s" << endl;
	cout << "[    Each    ] consume: " << t12.count() << "s" << endl;
	cout << "[ParallelEach] consume: " << t32.count() << "s" << endl;

	return 0;
}
