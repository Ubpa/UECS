#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

class RTDSystem: public System{
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
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
				[](RTDCmptsView cmpts) {
					for (auto handle : cmpts) {
						if (handle.GetCmptType() == CmptType{ "LuaCmpt" }) {
							double& val = *reinterpret_cast<double*>(handle.AsWrite().Ptr());
							val = 520.;
						}
					}
				}, "write", locator_write)
			.Register(
				[](RTDCmptsView cmpts) {
					for (auto handle : cmpts) {
						if (handle.GetCmptType() == CmptType{ "LuaCmpt" }) {
							const double& val = *reinterpret_cast<const double*>(handle.AsLatest().Ptr());
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
	w.entityMngr.Attach(e, &type, 1);

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
