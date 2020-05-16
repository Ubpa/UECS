#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

struct RTDSystem {
	static void OnUpdate(Schedule& schedule) {
		EntityLocator locator_write(
			{}, // read: last frame
			{ CmptType{ "LuaCmpt" } }, // write
			{} // read: lastest
		);
		EntityLocator locator_read(
			{}, // read: last frame
			{}, // write
			{ CmptType{ "LuaCmpt" } } // read: lastest
		);
		schedule
			.Register(
				[](const EntityLocator* locator, void** cmpts) {
					size_t i = 0;
					for (auto type : locator->CmptTypes()) {
						if (type == CmptType{ "LuaCmpt" }) {
							double& val = *reinterpret_cast<double*>(cmpts[i]);
							val = 520.;
						}
						i++;
					}
				}, "write", locator_write)
			.Register(
				[](const EntityLocator* locator, void** cmpts) {
					size_t i = 0;
					for (auto type : locator->CmptTypes()) {
						if (type == CmptType{ "LuaCmpt" }) {
							const double& val = *reinterpret_cast<double*>(cmpts[i]);
							cout << "value : " << val << endl;
						}
						i++;
					}
				}, "read", locator_read);
	}
};

int main() {
	CmptType type("LuaCmpt");
	// LuaCmpt {
	//   number value;
    // }
	RTDCmptTraits::Instance().RegisterSize(type, 8);
	RTDCmptTraits::Instance().RegisterDefaultConstructor(type, [](void*) { cout << "construct" << endl;});
	RTDCmptTraits::Instance().RegisterDestructor(type, [](void*) { cout << "destructor" << endl; });

	World w;
	w.systemMngr.Register<RTDSystem>();

	auto [e] = w.entityMngr.Create();
	w.entityMngr.Attach(e, type);

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
