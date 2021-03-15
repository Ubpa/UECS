#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct Data { std::size_t value; };

struct MySystem {
	static void OnUpdate(Schedule& schedule) {
		auto buffer = std::make_shared<std::vector<std::size_t>>();
		auto f = schedule.RegisterEntityJob(
			[buffer](std::size_t idxInQuery, const Data* data) {
				buffer->at(idxInQuery) = data->value;
			},
			"use"
		);
		schedule.RegisterJob([buffer]() {
			std::size_t sum = 0;
			for (std::size_t i : *buffer)
				sum += i;
				cout << sum << endl;
			},
			"print"
		);
		schedule.RegisterJob(
			[buffer, f](World* w) {
				std::size_t num = w->entityMngr.EntityNum(f->entityQuery);
				buffer->resize(num);
			},
			"allocate"
		);
		schedule.Order("allocate", "use");
		schedule.Order("use", "print");

	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<Data>();
	w.systemMngr.RegisterAndActivate<MySystem>();

	for (std::size_t i = 1; i <= 100; i++) {
		auto e = w.entityMngr.Create(Ubpa::TypeIDs_of<Data>);
		w.entityMngr.Get<Data>(e)->value = i;
	}

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
