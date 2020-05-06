#include <UECS/Entity.h>

using namespace Ubpa;
using namespace std;

vector<CmptPtr> Entity::Components() const {
	return archetype->Components(idx);
}

void Entity::Release() noexcept {
	archetype->mngr->Release(this);
}

void Entity::AddCommand(const std::function<void()>& command) {
	archetype->mngr->AddCommand(command);
}

Ubpa::World* Entity::World() const noexcept {
	return archetype->mngr->World();
}
