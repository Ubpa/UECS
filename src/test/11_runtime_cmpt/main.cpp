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
				[](RTDCmptViewer cmpts) {
					for (auto cmptptr : cmpts) {
						if (cmptptr.Type() == CmptType{ "LuaCmpt" }) {
							double& val = *reinterpret_cast<double*>(cmptptr.Ptr());
							val = 520.;
						}
					}
				}, "write", locator_write)
			.Register(
				[](RTDCmptViewer cmpts) {
					for (auto cmptptr : cmpts) {
						if (cmptptr.Type() == CmptType{ "LuaCmpt" }) {
							double& val = *reinterpret_cast<double*>(cmptptr.Ptr());
							cout << "value : " << val << endl;
						}
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
	RTDCmptTraits::Instance().RegisterDestructor(type, [](void*) { cout << "destruct" << endl; });

	World w;
	w.systemMngr.Register<RTDSystem>();

	auto [e] = w.entityMngr.Create();
	w.entityMngr.Attach(e, type);

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
