#include <UECS/CmptLocator.h>

#include <UECS/detail/Util.h>

#include <UContainer/Algorithm.h>

using namespace Ubpa::UECS;
using namespace std;

CmptLocator::CmptLocator(const CmptType* types, size_t num) {
	assert(types != nullptr && num > 0);
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

CmptLocator::CmptLocator()
	: hashCode{ TypeID<CmptLocator> } {}

size_t CmptLocator::GenHashCode() const noexcept {
	size_t rst = TypeID<CmptLocator>;
	for (auto type : cmptTypes)
		rst = hash_combine(rst, type.HashCode());
	return rst;
}

bool CmptLocator::operator==(const CmptLocator& rhs) const noexcept {
	return lastFrameCmptTypes == rhs.lastFrameCmptTypes
		&& writeCmptTypes == rhs.writeCmptTypes
		&& latestCmptTypes == rhs.latestCmptTypes;
}
