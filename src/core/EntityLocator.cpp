#include <UECS/EntityLocator.h>

#include <UECS/detail/Util.h>

using namespace Ubpa;

size_t EntityLocator::GenHashCode() const noexcept {
	size_t rst = TypeID<EntityLocator>;
	for (auto type : cmptTypes)
		rst = hash_combine(rst, type.HashCode());
	return rst;
}
