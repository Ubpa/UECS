#include <UECS/EntityLocator.h>

#include <UECS/detail/Util.h>

using namespace Ubpa::UECS;
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

AccessMode  EntityLocator::GetCmptTagMode(CmptType type) const {
	assert(cmptTypes.find(type) != cmptTypes.end());
	if (lastFrameCmptTypes.find(type) != lastFrameCmptTypes.end())
		return AccessMode::LAST_FRAME;
	else if (writeCmptTypes.find(type) != writeCmptTypes.end())
		return AccessMode::WRITE;
	else // lastestCmptTypes.find(type) != lastestCmptTypes.end())
		return AccessMode::LATEST;
}
