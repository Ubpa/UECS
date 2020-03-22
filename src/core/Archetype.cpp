#include <UECS/detail/Archetype.h>

using namespace std;
using namespace Ubpa;

Pool<Chunk> Archetype::chunkPool;

Archetype::~Archetype() {
	for (auto c : chunks)
		chunkPool.recycle(c);
}

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

	auto [size, offset] = target->second;
	size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();
	return { buffer + offset + idxInChunk * size, size };
}

tuple<size_t, vector<tuple<void*, void*>>> Archetype::Erase(size_t idx) {
	assert(idx < num);

	size_t movedIdx;
	vector<tuple<void*, void*>> src_dst;
	
	if (idx != num - 1) {
		movedIdx = num - 1;
		size_t dstIdxInChunk = idx % chunkCapacity;
		byte* dstBuffer = chunks[idx / chunkCapacity]->Data();
		size_t srcIdxInChunk = movedIdx % chunkCapacity;
		byte* srcBuffer = chunks[movedIdx / chunkCapacity]->Data();
		for (auto p : h2so) {
			auto [size, offset] = p.second;
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;
			src_dst.emplace_back(src, dst);
			memcpy(dst, src, size);
		}
	}
	else
		movedIdx = static_cast<size_t>(-1);

	num--;

	if (chunks.size() * chunkCapacity - num >= chunkCapacity) {
		Chunk* back = chunks.back();
		chunkPool.recycle(back);
	}

	return {movedIdx, src_dst};
}

vector<tuple<void*, size_t>> Archetype::Components(size_t idx) {
	vector<tuple<void*, size_t>> rst;

	for (const auto& [h, so] : h2so)
		rst.push_back(At(h, idx));

	return rst;
}
