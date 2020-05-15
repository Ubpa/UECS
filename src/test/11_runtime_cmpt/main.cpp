#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct RTDSystem {
	static void OnUpdate(Schedule& schedule) {
		EntityLocator locator(
			{ CmptType{ "LuaCmpt" } }, // read: last frame
			{}, // write
			{} // read: lastest
		);
		schedule.Register([](const EntityLocator* locator, void** cmpts) {
			for (auto type : locator->CmptTypes())
				cout << type.HashCode() << endl;
			}, "run-time dynamic func", locator);
	}
};

int main() {
	CmptType type("LuaCmpt");
	RTDCmptTraits::Instance().RegisterSize(type, 16);
	RTDCmptTraits::Instance().RegisterDefaultConstructor(type, [](void*) { cout << "construct" << endl;});
	RTDCmptTraits::Instance().RegisterDestructor(type, [](void*) { cout << "destructor" << endl; });

	World w;
	w.systemMngr.Register<RTDSystem>();

	auto [e] = w.entityMngr.CreateEntity();
	w.entityMngr.Attach(e, type);
	// C-style API
	//w.entityMngr.Attach(e, &type, 1);

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
