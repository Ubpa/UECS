#include <UECS/ArchetypeFilter.h>

#include <UECS/detail/Util.h>

using namespace Ubpa::UECS;

size_t ArchetypeFilter::HashCode() const noexcept {
	size_t rst = TypeID<ArchetypeFilter>;
	for (const auto& type : all)
		rst = hash_combine(rst, type.HashCode());
	for (const auto& type : any)
		rst = hash_combine(rst, type.HashCode());
	for (const auto& type : none)
		rst = hash_combine(rst, type.HashCode());
	return rst;
}

bool ArchetypeFilter::operator==(const ArchetypeFilter& rhs) const noexcept {
	return all == rhs.all
		&& any == rhs.any
		&& none == rhs.none;
}
