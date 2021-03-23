#include <UECS/UECS.hpp>

using namespace Ubpa::UECS;

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

struct A { float val; };
struct B { float val; };

struct TestSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](const A* a, B* b) {
				// 256 floating-point operations
				for (std::size_t i = 0; i < 256; i++)
					b->val *= a->val;
			}, "TestSystem");
	}
};

int main() {
	std::size_t numEntities = 65536;
	std::size_t numUpdate = 144 * 10;
	World w;
	w.entityMngr.cmptTraits.Register<A, B>();
	w.systemMngr.RegisterAndActivate<TestSystem>();

	{ // ECS
		auto t0 = std::chrono::high_resolution_clock::now();
		for (std::size_t i = 0; i < numEntities; i++)
			w.entityMngr.Create(Ubpa::TypeIDs_of<A, B>);
		auto t1 = std::chrono::high_resolution_clock::now();
		for (std::size_t i = 0; i < numUpdate; i++)
			w.Update();
		auto t2 = std::chrono::high_resolution_clock::now();

		// G5400 : 2 cores 4 threads
		// about 10s

		// i5 8400 : 6 cores 6 threads
		// about 6s

		// i5 10400 : 6 cores 12 threads
		// about 2.7s

		auto d0 = t1 - t0;
		auto d1 = t2 - t1;
		std::cout << "create: " << d0.count() / 1000000000.0 << "s" << std::endl;
		std::cout << "update: " << d1.count() / 1000000000.0 << "s" << std::endl;
	}
	{ // direct
		auto t0 = std::chrono::high_resolution_clock::now();
		std::vector<std::unique_ptr<A>> As;
		std::vector<std::unique_ptr<B>> Bs;

		for (std::size_t i = 0; i < numEntities; i++) {
			auto a = std::make_unique<A>();
			a->val = 1;
			auto b = std::make_unique<B>();
			b->val = 2;
			As.push_back(std::move(a));
			Bs.push_back(std::move(b));
		}
		auto t1 = std::chrono::high_resolution_clock::now();
		size_t N = std::thread::hardware_concurrency();
		for (std::size_t i = 0; i < numUpdate; i++) {
			auto work = [&](std::size_t id) {
				for (std::size_t j = id; j < numEntities; j += N) {
					const auto& a = As[j];
					const auto& b = Bs[j];
					for (std::size_t k = 0; k < 256; k++) {
						b->val *= a->val;
					}
				}
			};
			std::vector<std::thread> jobs;
			for (std::size_t n = 0; n < N; n++)
				jobs.emplace_back(work, n);
			for (auto& job : jobs)
				job.join();
		}
		auto t2 = std::chrono::high_resolution_clock::now();

		// i5 10400 : 6 cores 12 threads
		// about 13s

		auto d0 = t1 - t0;
		auto d1 = t2 - t1;
		std::cout << "create: " << d0.count() / 1000000000.0 << "s" << std::endl;
		std::cout << "update: " << d1.count() / 1000000000.0 << "s" << std::endl;
	}

	return 0;
}
