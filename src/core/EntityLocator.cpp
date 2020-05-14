#include <UECS/EntityLocator.h>

#include <UECS/detail/Util.h>

using namespace Ubpa;
using namespace std;

EntityLocator::EntityLocator(set<CmptType> lastFrameCmpts,
	set<CmptType> writeFrameCmpts,
	set<CmptType> latestCmpts)
	: lastFrameCmptTypes{ move(lastFrameCmpts) },
	writeCmptTypes{ move(writeFrameCmpts) },
	latestCmptTypes{ move(latestCmpts) }
{
	cmptTypes = SetUnion(lastFrameCmptTypes, writeCmptTypes);
	cmptTypes = SetUnion(cmptTypes, latestCmptTypes);
	hashCode = GenHashCode();
}

size_t EntityLocator::GenHashCode() const noexcept {
	size_t rst = TypeID<EntityLocator>;
	for (auto type : cmptTypes)
		rst = hash_combine(rst, type.HashCode());
	return rst;
}

bool EntityLocator::operator==(const EntityLocator& locator) const noexcept {
	return lastFrameCmptTypes == locator.lastFrameCmptTypes
		&& writeCmptTypes == locator.writeCmptTypes
		&& latestCmptTypes == latestCmptTypes;
}
