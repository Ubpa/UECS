#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

class RTDSystem: public System{
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		std::array<CmptType, 1> cmpts_write = {
			CmptType{ "LuaCmpt", AccessMode::WRITE }
		};
		std::array<CmptType, 1> cmpts_read = {
			CmptType{ "LuaCmpt", AccessMode::LATEST }
		};

		EntityLocator locator_write(
			cmpts_write.data(), cmpts_write.size()
		);
		EntityLocator locator_read(
			cmpts_read.data(), cmpts_read.size()
		);

		schedule.Register(
			[](CmptsView cmpts) {
				auto luaCmpt = cmpts.GetCmpt(CmptType{ "LuaCmpt", AccessMode::WRITE });
				double& val = *reinterpret_cast<double*>(luaCmpt.Ptr());
				val = 520.;
			}, "write", locator_write);
		schedule.Register(
			[](CmptsView cmpts) {
				auto luaCmpt = cmpts.GetCmpt(CmptType{ "LuaCmpt", AccessMode::LATEST });
				const double& val = *reinterpret_cast<const double*>(luaCmpt.Ptr());
				cout << "value : " << val << endl;
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
