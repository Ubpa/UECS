#include <UECS/World.h>

#include <iostream>

using namespace Ubpa;
using namespace std;

int main() {
	CmptType type("LuaCmpt A");
	RTDCmptTraits::Instance().RegisterSize(type, 16);

	World w;
	w.entityMngr.CreateEntity(type);
	return 0;
}
