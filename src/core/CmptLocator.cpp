#include <UECS/CmptLocator.h>

#include <UECS/detail/Util.h>

using namespace Ubpa::UECS;

CmptLocator::CmptLocator(const CmptAccessType* types, size_t num) {
	assert(types || num == 0);
	for (size_t i = 0; i < num; i++) {
		assert(!AccessMode_IsSingleton(types[i].GetAccessMode()));
		cmptTypes.insert(types[i]);
	}

	hashCode = GenHashCode();
}

CmptLocator::CmptLocator()
	: hashCode{ TypeID<CmptLocator> } {}

size_t CmptLocator::GenHashCode() const noexcept {
	size_t rst = TypeID<CmptLocator>;
	for (const auto& type : cmptTypes)
		rst = hash_combine(rst, type.HashCode());
	return rst;
}

bool CmptLocator::HasWriteCmptType() const noexcept {
	for (const auto& type : cmptTypes) {
		if (type.GetAccessMode() == AccessMode::WRITE)
			return true;
	}
	return false;
}

bool CmptLocator::operator==(const CmptLocator& rhs) const {
	return cmptTypes == rhs.cmptTypes;
}
