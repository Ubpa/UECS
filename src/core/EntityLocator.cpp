#include <UECS/EntityLocator.h>

#include <UECS/detail/Util.h>

using namespace Ubpa::UECS;
using namespace std;

EntityLocator::EntityLocator(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++) {
		switch (types[i].GetAccessMode())
		{
		case Ubpa::UECS::AccessMode::LAST_FRAME:
			lastFrameCmptTypes.insert(types[i]);
			break;
		case Ubpa::UECS::AccessMode::WRITE:
			writeCmptTypes.insert(types[i]);
			break;
		case Ubpa::UECS::AccessMode::LATEST:
			latestCmptTypes.insert(types[i]);
			break;
		default:
			assert(false);
			break;
		}
	}
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
