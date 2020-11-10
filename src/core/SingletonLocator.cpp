#include <UECS/SingletonLocator.h>

using namespace Ubpa::UECS;
using namespace std;

SingletonLocator::SingletonLocator(const CmptAccessType* types, size_t num) {
	assert(types || num == 0);
	for (size_t i = 0; i < num; i++) {
		singletonTypes.insert(types[i]);
	}
}

bool SingletonLocator::HasWriteSingletonType() const noexcept {
	for (const auto& type : singletonTypes) {
		if (type.GetAccessMode() == AccessMode::WRITE)
			return true;
	}
	return false;
}
