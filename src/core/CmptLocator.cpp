#include <UECS/CmptLocator.h>

#include <UECS/detail/Util.h>

#include <UContainer/Algorithm.h>

using namespace Ubpa::UECS;
using namespace std;

CmptLocator::CmptLocator(const CmptType* types, size_t num) {
	assert(types != nullptr && num > 0);
	for (size_t i = 0; i < num; i++)
		cmptTypes.insert(types[i]);

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

bool CmptLocator::operator==(const CmptLocator& rhs) const noexcept {
	return cmptTypes == rhs.cmptTypes;
}
