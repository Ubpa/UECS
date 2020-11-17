#include <UECS/World.h>

using namespace Ubpa::UECS;

#include <chrono>
#include <iostream>

struct A { float val; };
struct B { float val; };

struct TestSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](const A* a, B* b) {
				// 256 floating-point operations
				for (size_t i = 0; i < 256; i++)
					b->val *= a->val;
			}, "TestSystem");
	}
};

int main() {
	size_t numEntities = 65536;
	size_t numUpdate = 144 * 10;
	World w;
	w.systemMngr.RegisterAndActivate<TestSystem>();

	auto t0 = std::chrono::steady_clock::now();
	for (size_t i = 0; i < numEntities; i++)
		w.entityMngr.Create<A, B>();
	auto t1 = std::chrono::steady_clock::now();
	for (size_t i = 0; i < numUpdate; i++)
		w.Update();
	auto t2 = std::chrono::steady_clock::now();

	// G5400 : 4 cores
	// about 10s
	
	// i5 8400 : 6 cores
	// about 6s
	auto d0 = t1 - t0;
	auto d1 = t2 - t1;
	std::cout << "create: " << d0.count() / 1000000000.0 << "s" << std::endl;
	std::cout << "update: " << d1.count() / 1000000000.0 << "s" << std::endl;

	return 0;
}
