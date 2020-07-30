#include <UECS/EntityFilter.h>

using namespace Ubpa::UECS;

EntityFilter::EntityFilter()
	: allHashCode{ TypeID<EntityFilter> },
	anyHashCode{ TypeID<EntityFilter> },
	noneHashCode{ TypeID<EntityFilter> },
	combinedHashCode{ hash_combine(std::array<size_t, 3>{TypeID<EntityFilter>, TypeID<EntityFilter>, TypeID<EntityFilter>}) }
{
}

EntityFilter::EntityFilter(std::set<CmptType> allCmptTypes,
	std::set<CmptType> anyCmptTypes,
	std::set<CmptType> noneCmptTypes)
	: allCmptTypes{ move(allCmptTypes) },
	anyCmptTypes{ move(anyCmptTypes) },
	noneCmptTypes{ move(noneCmptTypes) },
	allHashCode{ GenAllHashCode() },
	anyHashCode{ GenAnyHashCode() },
	noneHashCode{ GenNoneHashCode() },
	combinedHashCode{ GenCombinedHashCode()}
{
}

void EntityFilter::InsertAll(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		allCmptTypes.insert(types[i]);
	allHashCode = GenAllHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void EntityFilter::InsertAny(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		anyCmptTypes.insert(types[i]);
	anyHashCode = GenAnyHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void EntityFilter::InsertNone(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		noneCmptTypes.insert(types[i]);
	noneHashCode = GenNoneHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void EntityFilter::EraseAll(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		allCmptTypes.erase(types[i]);
	allHashCode = GenAllHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void EntityFilter::EraseAny(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
	anyHashCode = GenAnyHashCode();
	combinedHashCode = GenCombinedHashCode();
}

void EntityFilter::EraseNone(const CmptType* types, size_t num) {
	assert(types != nullptr);
	for (size_t i = 0; i < num; i++)
		noneCmptTypes.erase(types[i]);
	noneHashCode = GenNoneHashCode();
	combinedHashCode = GenCombinedHashCode();
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

size_t EntityFilter::GenCombinedHashCode() const noexcept {
	return hash_combine(std::array<size_t, 3>{allHashCode, anyHashCode, noneHashCode});
}

bool EntityFilter::operator==(const EntityFilter& filter) const noexcept {
	return allCmptTypes == filter.allCmptTypes
		&& anyCmptTypes == filter.anyCmptTypes
		&& noneCmptTypes == filter.noneCmptTypes;
}
