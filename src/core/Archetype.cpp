#include <UECS/core/Archetype.h>

using namespace std;
using namespace Ubpa;

pool<Chunk> Archetype::chunkPool;

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

pair<void*, size_t> Archetype::At(size_t cmptHash, size_t idx) {
	auto target = h2so.find(cmptHash);
	if (target == h2so.end())
		return { nullptr,static_cast<size_t>(-1) };

	size_t size = target->second.first;
	size_t offset = target->second.second;
	size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();
	return { buffer + offset + size * idxInChunk, size };
}

pair<size_t, vector<pair<void*, void*>>> Archetype::Erase(size_t idx) {
	assert(idx < num);
	pair<size_t, vector<pair<void*, void*>>> rst;
	
	if (idx != num - 1) {
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();
		for (auto p : h2so) {
			size_t size = p.second.first;
			size_t offset = p.second.second;
			byte* dst = buffer + offset + size * idxInChunk;
			byte* src = buffer + offset + (num - 1) * idxInChunk;
			rst.second.emplace_back(dst, src);
			memcpy(dst, src, size);
		}
		rst.first = num - 1;
	}
	else
		rst.first = static_cast<size_t>(-1);

	num--;

	if (chunks.size() * chunkCapacity - num >= chunkCapacity) {
		Chunk* back = chunks.back();
		chunkPool.recycle(back);
	}

	return rst;
}
