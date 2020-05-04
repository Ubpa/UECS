#include <UECS/Entity.h>

using namespace Ubpa;
using namespace std;

vector<CmptPtr> Entity::Components() const {
	return archetype->Components(idx);
}

void Entity::Release() noexcept {
	assert(IsAlive());
	archetype->mngr->Release(this);
}

bool Entity::IsAlive() const noexcept {
	return archetype != nullptr;
}

void Entity::AddCommand(const std::function<void()>& command) {
	archetype->mngr->AddCommand(command);
}

Ubpa::World* Entity::World() const noexcept {
	assert(IsAlive());
	return archetype->mngr->World();
}
