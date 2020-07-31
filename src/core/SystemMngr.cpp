#include <UECS/SystemMngr.h>

#include <UECS/IListener.h>

using namespace Ubpa::UECS;

void SystemMngr::Accept(IListener* listener) const {
	listener->EnterSystemMngr(this);
	for (const auto& [n, system] : systems) {
		listener->EnterSystem(system.get());
		listener->ExistSystem(system.get());
	}
	listener->ExistSystemMngr(this);
}
