#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

int main() {
	CmptType type("LuaCmpt");
	RTDCmptTraits::Instance().RegisterSize(type, 16);
	RTDCmptTraits::Instance().RegisterDefaultConstructor(type, [](void*) { cout << "construct" << endl;});
	RTDCmptTraits::Instance().RegisterDestructor(type, [](void*) { cout << "destructor" << endl; });

	World w;
	auto [e] = w.entityMngr.CreateEntity();
	auto [cmpt] = w.entityMngr.Attach(e, type);

	return 0;
}
