#include <UECS/Archetype.h>

using namespace std;
using namespace Ubpa;

bool Archetype::ID::operator<(const ID& id) const noexcept {
	auto l = begin(), r = id.begin();
	while (l != end() && r != id.end()) {
		if (*l < *r)
			return true;
		if (*l > * r)
			return false;
		++l;
		++r;
	}
	return l == end() && r != id.end();
}

bool Archetype::ID::operator==(const ID& id) const noexcept {
	if (size() != id.size())
		return false;
	for (auto l = begin(), r = id.begin(); l != end(); ++l, ++r) {
		if (*l != *r)
			return false;
	}
	return true;
}

tuple<void*, size_t> Archetype::At(size_t cmptHash, size_t idx) {
	auto target = h2so.find(cmptHash);
	if (target == h2so.end())
		return { nullptr,static_cast<size_t>(-1) };

	size_t size = get<0>(target->second);
	size_t offset = get<1>(target->second);
	size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();
	return { buffer + offset + size * idxInChunk, size };
}

size_t Archetype::Erase(size_t idx) {
	assert(idx < num);
	size_t movedIdx;
	if (idx != num - 1) {
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();
		for (auto p : h2so) {
			size_t size = std::get<0>(p.second);
			size_t offset = std::get<1>(p.second);
			byte* dst = buffer + offset + size * idxInChunk;
			byte* src = buffer + offset + (num - 1) * idxInChunk;
			memcpy(dst, src, size);
		}
		movedIdx = num - 1;
	}
	else
		movedIdx = static_cast<size_t>(-1);

	num--;

	return movedIdx;
}
