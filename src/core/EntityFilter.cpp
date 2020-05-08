#include <UECS/EntityFilter.h>

using namespace Ubpa;

EntityFilter::EntityFilter()
	: allHashCode{ TypeID<EntityFilter> },
	anyHashCode{ TypeID<EntityFilter> },
	noneHashCode{ TypeID<EntityFilter> },
	combinedHashCode{ hash_combine(std::array<size_t, 3>{TypeID<EntityFilter>, TypeID<EntityFilter>, TypeID<EntityFilter>}) }
{
}

size_t EntityFilter::GenAllHashCode() const noexcept {
	size_t rst = TypeID<EntityFilter>;
	for (auto type : allCmptTypes) {
		rst = hash_combine(rst, type.HashCode());
	}
	return rst;
}

size_t EntityFilter::GenAnyHashCode() const noexcept {
	size_t rst = TypeID<EntityFilter>;
	for (auto type : anyCmptTypes) {
		rst = hash_combine(rst, type.HashCode());
	}
	return rst;
}

size_t EntityFilter::GenNoneHashCode() const noexcept {
	size_t rst = TypeID<EntityFilter>;
	for (auto type : noneCmptTypes) {
		rst = hash_combine(rst, type.HashCode());
	}
	return rst;
}
