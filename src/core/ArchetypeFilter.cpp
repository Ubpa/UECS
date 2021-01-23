#include <UECS/ArchetypeFilter.h>

#include <UECS/details/Util.h>

using namespace Ubpa::UECS;

std::size_t ArchetypeFilter::GetValue() const noexcept {
	std::size_t rst = TypeID_of<ArchetypeFilter>.GetValue();
	for (const auto& type : all)
		rst = hash_combine(rst, type.GetValue());
	for (const auto& type : any)
		rst = hash_combine(rst, type.GetValue());
	for (const auto& type : none)
		rst = hash_combine(rst, type.GetValue());
	return rst;
}

bool ArchetypeFilter::HaveWriteTypeID() const noexcept {
	for (const auto& type : all) {
		if (type.GetAccessMode() == AccessMode::WRITE)
			return true;
	}
	for (const auto& type : any) {
		if (type.GetAccessMode() == AccessMode::WRITE)
			return true;
	}
	return false;
}

bool ArchetypeFilter::operator==(const ArchetypeFilter& rhs) const noexcept {
	return all == rhs.all
		&& any == rhs.any
		&& none == rhs.none;
}
