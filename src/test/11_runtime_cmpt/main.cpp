#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

class RTDSystem: public System{
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		std::array cmpts_write = {
			CmptAccessType{ "LuaCmpt", AccessMode::WRITE }
		};
		std::array cmpts_read = {
			CmptAccessType{ "LuaCmpt", AccessMode::LATEST }
		};

		CmptLocator locator_write(
			cmpts_write.data(), cmpts_write.size()
		);
		CmptLocator locator_read(
			cmpts_read.data(), cmpts_read.size()
		);

		schedule.RegisterEntityJob(
			[](CmptsView cmpts) {
				auto luaCmpt = cmpts.GetCmpt(CmptAccessType{ "LuaCmpt", AccessMode::WRITE });
				double& val = *reinterpret_cast<double*>(luaCmpt.Ptr());
				val = 520.;
			}, "write", ArchetypeFilter{}, locator_write);
		schedule.RegisterEntityJob(
			[](CmptsView cmpts) {
				auto luaCmpt = cmpts.GetCmpt(CmptAccessType{ "LuaCmpt", AccessMode::LATEST });
				const double& val = *reinterpret_cast<const double*>(luaCmpt.Ptr());
				cout << "value : " << val << endl;
			}, "read", ArchetypeFilter{}, locator_read);
	}
};

int main() {
	CmptType type("LuaCmpt");
	// LuaCmpt {
	//   number value;
    // }

	World w;
	w.systemMngr.Register<RTDSystem>();
	w.entityMngr.cmptTraits
		.RegisterSize(type, 8)
		.RegisterDefaultConstructor(type, [](void*) { cout << "construct" << endl; })
		.RegisterDestructor(type, [](void*) { cout << "destruct" << endl; });

	auto [e] = w.entityMngr.Create();
	w.entityMngr.Attach(e, &type, 1);

	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
