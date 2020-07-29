#include <UECS/SystemMngr.h>

#include <UECS/IListener.h>

using namespace Ubpa;

void SystemMngr::Accept(IListener* listener) const {
	listener->EnterSystemMngr(this);
	for (const auto& [n, f] : onUpdateMap) {
		listener->EnterSystem(n);
		listener->ExistSystem(n);
	}
	listener->ExistSystemMngr(this);
}
