#include <UECS/detail/Archetype.h>

#include <cstdlib>

using namespace std;
using namespace Ubpa;

Archetype::~Archetype() {
	for (auto chunk : chunks) {
#ifdef WIN32
		_aligned_free(chunk);
#else
		free(chunk);
#endif // WIN32
	}
}

size_t Archetype::RequestBuffer() {
	if (num == chunks.size() * chunkCapacity) {
#ifdef WIN32
		auto chunk = reinterpret_cast<Chunk*>(_aligned_malloc(sizeof(Chunk), std::alignment_of_v<Chunk>));
#else
		auto chunk = reinterpret_cast<Chunk*>(aligned_alloc(sizeof(Chunk), std::alignment_of_v<Chunk>));
#endif // WIN32
		chunks.push_back(chunk);
	}
	return num++;
}

void* Archetype::At(size_t cmptID, size_t idx) const {
	auto target = id2so.find(cmptID);
	if (target == id2so.end())
		return nullptr;

	auto [size, offset] = target->second;
	size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();

	return buffer + offset + idxInChunk * size;
}

size_t Archetype::Erase(size_t idx) {
	assert(idx < num);

	size_t movedIdx;
	
	if (idx != num - 1) {
		movedIdx = num - 1;

		size_t dstIdxInChunk = idx % chunkCapacity;
		byte* dstBuffer = chunks[idx / chunkCapacity]->Data();
		size_t srcIdxInChunk = movedIdx % chunkCapacity;
		byte* srcBuffer = chunks[movedIdx / chunkCapacity]->Data();

		for (auto [id, so] : id2so) {
			auto [size, offset] = so;
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;
			CmptLifecycleMngr::Instance().Destruct(id, dst);
			CmptLifecycleMngr::Instance().MoveConstruct(id, dst, src);
		}
	}
	else
		movedIdx = static_cast<size_t>(-1);

	num--;

	if (chunks.size() * chunkCapacity - num >= chunkCapacity) {
		Chunk* chunk = chunks.back();
#ifdef WIN32
		_aligned_free(chunk);
#else
		free(chunk);
#endif // WIN32
		chunks.pop_back();
	}

	return movedIdx;
}

vector<CmptPtr> Archetype::Components(size_t idx) const {
	vector<CmptPtr> rst;

	for (const auto& [id, so] : id2so)
		rst.emplace_back(id, At(id, idx));

	return rst;
}
