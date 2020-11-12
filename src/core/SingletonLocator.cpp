#include <UECS/SingletonLocator.h>

using namespace Ubpa::UECS;
using namespace std;

SingletonLocator::SingletonLocator(Span<const CmptAccessType> types) {
	for (const auto& type : types)
		singletonTypes.insert(type);
}

bool SingletonLocator::HasWriteSingletonType() const noexcept {
	for (const auto& type : singletonTypes) {
		if (type.GetAccessMode() == AccessMode::WRITE)
			return true;
	}
	return false;
}
