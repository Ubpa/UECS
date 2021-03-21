#include <UECS/UECS.hpp>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UECS;

struct Data { float value; };
struct DoubleData { float value; };

struct DataSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](const Data* d, DoubleData* dd) {
				std::cout << "Doublize" << std::endl;
				dd->value = 2 * d->value;
			},
			"Doublize",
			Schedule::EntityJobConfig{
				.changeFilter = {{TypeID_of<Data>}}
			}
		);
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<Data, DoubleData>();
	w.systemMngr.RegisterAndActivate<DataSystem>();
	Entity e = w.entityMngr.Create(TypeIDs_of<Data, DoubleData>);

	w.entityMngr.WriteComponent<Data>(e)->value = 2;
	std::cout << "Data: " << w.entityMngr.ReadComponent<Data>(e)->value << std::endl
		<< "DoubleData: " << w.entityMngr.ReadComponent<DoubleData>(e)->value << std::endl;
	w.Update(); // Doublize
	std::cout << "Data: " << w.entityMngr.ReadComponent<Data>(e)->value << std::endl
		<< "DoubleData: " << w.entityMngr.ReadComponent<DoubleData>(e)->value << std::endl;
	w.Update(); // not Doublize
	std::cout << "Data: " << w.entityMngr.ReadComponent<Data>(e)->value << std::endl
		<< "DoubleData: " << w.entityMngr.ReadComponent<DoubleData>(e)->value << std::endl;
	w.entityMngr.WriteComponent<Data>(e)->value = 3;
	std::cout << "Data: " << w.entityMngr.ReadComponent<Data>(e)->value << std::endl
		<< "DoubleData: " << w.entityMngr.ReadComponent<DoubleData>(e)->value << std::endl;
	w.Update(); // Doublize
	std::cout << "Data: " << w.entityMngr.ReadComponent<Data>(e)->value << std::endl
		<< "DoubleData: " << w.entityMngr.ReadComponent<DoubleData>(e)->value << std::endl;
	w.Update(); // not Doublize
	std::cout << "Data: " << w.entityMngr.ReadComponent<Data>(e)->value << std::endl
		<< "DoubleData: " << w.entityMngr.ReadComponent<DoubleData>(e)->value << std::endl;

	return 0;
}
