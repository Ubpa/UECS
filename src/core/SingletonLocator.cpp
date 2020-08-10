#include <UECS/SingletonLocator.h>

#include <UECS/detail/Util.h>

#include <UContainer/Algorithm.h>

using namespace Ubpa::UECS;
using namespace std;

SingletonLocator::SingletonLocator(const CmptType* types, size_t num) {
	assert(types || num == 0);
	for (size_t i = 0; i < num; i++)
		singletonTypes.insert(types[i]);
}

bool SingletonLocator::HasWriteSingletonType() const noexcept {
	for (const auto& type : singletonTypes) {
		if (type.GetAccessMode() == AccessMode::WRITE_SINGLETON)
			return true;
	}
	return false;
}
