#include <UECS/ArchetypeFilter.h>

#include <UECS/detail/Util.h>

using namespace Ubpa::UECS;

size_t ArchetypeFilter::HashCode() const noexcept {
	size_t rst = TypeID<ArchetypeFilter>;
	for (const auto& type : all) {
		assert(!AccessMode_IsSingleton(type.GetAccessMode()));
		rst = hash_combine(rst, type.HashCode());
	}
	for (const auto& type : any) {
		assert(!AccessMode_IsSingleton(type.GetAccessMode()));
		rst = hash_combine(rst, type.HashCode());
	}
	for (const auto& type : none)
		rst = hash_combine(rst, type.HashCode());
	return rst;
}

bool ArchetypeFilter::HaveWriteCmptType() const noexcept {
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

bool ArchetypeFilter::operator==(const ArchetypeFilter& rhs) const {
	return all == rhs.all
		&& any == rhs.any
		&& none == rhs.none;
}
