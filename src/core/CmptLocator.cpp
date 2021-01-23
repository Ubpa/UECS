#include <UECS/CmptLocator.h>

#include <UECS/details/Util.h>

using namespace Ubpa::UECS;

CmptLocator::CmptLocator(AccessTypeIDSet cmptTypes)
	: cmptTypes{ std::move(cmptTypes) }
{
	UpdateGetValue();
}

CmptLocator::CmptLocator(std::span<const AccessTypeID> types) {
	for (const auto& type : types)
		cmptTypes.insert(type);

	UpdateGetValue();
}

CmptLocator::CmptLocator()
	: hashCode{ TypeID_of<CmptLocator>.GetValue() } {}

void CmptLocator::UpdateGetValue() noexcept {
	std::size_t rst = TypeID_of<CmptLocator>.GetValue();
	for (const auto& type : cmptTypes)
		rst = hash_combine(rst, type.GetValue());
	hashCode = rst;
}

bool CmptLocator::HasWriteTypeID() const noexcept {
	for (const auto& type : cmptTypes) {
		if (type.GetAccessMode() == AccessMode::WRITE)
			return true;
	}
	return false;
}

bool CmptLocator::operator==(const CmptLocator& rhs) const noexcept {
	return cmptTypes == rhs.cmptTypes;
}
