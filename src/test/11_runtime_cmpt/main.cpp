#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UECS;
using namespace std;

struct RTDSystem {
	static void OnUpdate(Schedule& schedule) {
		std::array cmpts_write = {
			AccessTypeID{ "LuaCmpt", AccessMode::WRITE }
		};
		std::array cmpts_read = {
			AccessTypeID{ "LuaCmpt", AccessMode::LATEST }
		};

		CmptLocator locator_write(cmpts_write);
		CmptLocator locator_read(cmpts_read);

		schedule.RegisterEntityJob(
			[](CmptsView cmpts) {
				auto luaCmpt = cmpts.GetCmpt(AccessTypeID{ "LuaCmpt", AccessMode::WRITE });
				double& val = *reinterpret_cast<double*>(luaCmpt.Ptr());
				val = 520.;
			},
			"write",
			true,
			ArchetypeFilter{},
			locator_write
		);
		schedule.RegisterEntityJob(
			[](CmptsView cmpts) {
				auto luaCmpt = cmpts.GetCmpt(AccessTypeID{ "LuaCmpt", AccessMode::LATEST });
				const double& val = *reinterpret_cast<const double*>(luaCmpt.Ptr());
				cout << "value : " << val << endl;
			},
			"read",
			true,
			ArchetypeFilter{},
			locator_read
		);
	}
};

int main() {
	TypeID type("LuaCmpt");
	// LuaCmpt {
	//   number value;
    // }

	World w;
	w.systemMngr.RegisterAndActivate<RTDSystem>();
	w.entityMngr.cmptTraits
		.RegisterSize(type, 8)
		.RegisterDefaultConstructor(type, [](void*) { cout << "construct" << endl; })
		.RegisterDestructor(type, [](void*) { cout << "destruct" << endl; });

	auto [e] = w.entityMngr.Create();
	w.entityMngr.Attach(e, { &type, 1 });
	w.Update();

	cout << w.DumpUpdateJobGraph() << endl;

	return 0;
}
