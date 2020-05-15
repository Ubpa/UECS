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
							double& v0 = *reinterpret_cast<double*>(cmpts[i]);
							double& v1 = *(reinterpret_cast<double*>(cmpts[i]) + 1);
							v0 = 1.;
							v1 = 2.;
						}
						i++;
					}
				}, "write", locator_write)
			.Register(
				[](const EntityLocator* locator, void** cmpts) {
					size_t i = 0;
					for (auto type : locator->CmptTypes()) {
						if (type == CmptType{ "LuaCmpt" }) {
							const double& v0 = *reinterpret_cast<double*>(cmpts[i]);
							const double& v1 = *(reinterpret_cast<double*>(cmpts[i]) + 1);
							cout
								<< "v0 : " << v0 << endl
								<< "v1 : " << v1 << endl;
						}
						i++;
					}
				}, "read", locator_read);
	}
};

int main() {
	CmptType type("LuaCmpt");
	// struct LuaCmpt {
	//   double value0;
	//   double value1;
    // }
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
