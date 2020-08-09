#include <UECS/ArchetypeFilter.h>

using namespace Ubpa::UECS;

ArchetypeFilter::ArchetypeFilter()
	:
	allHashCode{ TypeID<ArchetypeFilter> },
	anyHashCode{ TypeID<ArchetypeFilter> },
	noneHashCode{ TypeID<ArchetypeFilter> },
	combinedHashCode{ hash_combine(std::array<size_t, 3>{TypeID<ArchetypeFilter>, TypeID<ArchetypeFilter>, TypeID<ArchetypeFilter>}) }
{
}

ArchetypeFilter::ArchetypeFilter(
	std::set<CmptType> allCmptTypes,
	std::set<CmptType> anyCmptTypes,
	std::set<CmptType> noneCmptTypes)
	:
	allCmptTypes{ move(allCmptTypes) },
	anyCmptTypes{ move(anyCmptTypes) },
	noneCmptTypes{ move(noneCmptTypes) },
	allHashCode{ GenAllHashCode() },
	anyHashCode{ GenAnyHashCode() },
	noneHashCode{ GenNoneHashCode() },
	combinedHashCode{ GenCombinedHashCode()}
{
}

void ArchetypeFilter::InsertAll(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		allCmptTypes.insert(types[i]);
	allHashCode = GenAllHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void ArchetypeFilter::InsertAny(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		anyCmptTypes.insert(types[i]);
	anyHashCode = GenAnyHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void ArchetypeFilter::InsertNone(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		noneCmptTypes.insert(types[i]);
	noneHashCode = GenNoneHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void ArchetypeFilter::EraseAll(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		allCmptTypes.erase(types[i]);
	allHashCode = GenAllHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void ArchetypeFilter::EraseAny(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
	anyHashCode = GenAnyHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void ArchetypeFilter::EraseNone(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		noneCmptTypes.erase(types[i]);
	noneHashCode = GenNoneHashCode();
	combinedHashCode = GenCombinedHashCode();
}

size_t ArchetypeFilter::GenAllHashCode() const noexcept {
	size_t rst = TypeID<ArchetypeFilter>;
	for (auto type : allCmptTypes) {
		rst = hash_combine(rst, type.HashCode());
	}
	return rst;
}

size_t ArchetypeFilter::GenAnyHashCode() const noexcept {
	size_t rst = TypeID<ArchetypeFilter>;
	for (auto type : anyCmptTypes) {
		rst = hash_combine(rst, type.HashCode());
	}
	return rst;
}

size_t ArchetypeFilter::GenNoneHashCode() const noexcept {
	size_t rst = TypeID<ArchetypeFilter>;
	for (auto type : noneCmptTypes) {
		rst = hash_combine(rst, type.HashCode());
	}
	return rst;
}

size_t ArchetypeFilter::GenCombinedHashCode() const noexcept {
	return hash_combine(std::array<size_t, 3>{allHashCode, anyHashCode, noneHashCode});
}

bool ArchetypeFilter::operator==(const ArchetypeFilter& rhs) const noexcept {
	return allCmptTypes == rhs.allCmptTypes
		&& anyCmptTypes == rhs.anyCmptTypes
		&& noneCmptTypes == rhs.noneCmptTypes;
}
