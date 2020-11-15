#include <UECS/CmptLocator.h>

#include <UECS/detail/Util.h>

using namespace Ubpa::UECS;

CmptLocator::CmptLocator(CmptAccessTypeSet cmptTypes)
	: cmptTypes{ std::move(cmptTypes) }
{
	UpdateHashCode();
}

CmptLocator::CmptLocator(Span<const CmptAccessType> types) {
	for (const auto& type : types)
		cmptTypes.insert(type);

	UpdateHashCode();
}

CmptLocator::CmptLocator()
	: hashCode{ TypeID<CmptLocator> } {}

void CmptLocator::UpdateHashCode() noexcept {
	size_t rst = TypeID<CmptLocator>;
	for (const auto& type : cmptTypes)
		rst = hash_combine(rst, type.HashCode());
	hashCode = rst;
}

bool CmptLocator::HasWriteCmptType() const noexcept {
	for (const auto& type : cmptTypes) {
		if (type.GetAccessMode() == AccessMode::WRITE)
			return true;
	}
	return false;
}

bool CmptLocator::operator==(const CmptLocator& rhs) const noexcept {
	return cmptTypes == rhs.cmptTypes;
}
