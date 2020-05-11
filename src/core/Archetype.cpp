#include <UECS/detail/Archetype.h>

#include <cstdlib>

using namespace std;
using namespace Ubpa;

void Archetype::SetLayout() {
	std::vector<size_t> alignments;
	std::vector<size_t> sizes;

	for (const auto& type : types) {
		alignments.push_back(type2alignment[type]);
		sizes.push_back(type2size[type]);
	}

	auto layout = Chunk::GenLayout(alignments, sizes);
	chunkCapacity = layout.capacity;

	size_t i = 0;
	for (const auto& type : types)
		type2offset[type] = layout.offsets[i++];
}

Archetype::~Archetype() {
	for (size_t i = 0; i < entityNum; i++) {
		size_t idxInChunk = i % chunkCapacity;
		byte* buffer = chunks[i / chunkCapacity]->Data();
		for (const auto& type : types) {
			size_t size = type2size[type];
			size_t offset = type2offset[type];
			byte* address = buffer + offset + idxInChunk * size;
			RuntimeCmptTraits::Instance().Destruct(type, address);
		}
	}
	for (Chunk* chunk : chunks) {
#ifdef WIN32
		_aligned_free(chunk);
#else
		free(chunk);
#endif // WIN32
	}
}

size_t Archetype::RequestBuffer() {
	if (entityNum == chunks.size() * chunkCapacity) {
#ifdef WIN32
		auto chunk = reinterpret_cast<Chunk*>(_aligned_malloc(sizeof(Chunk), std::alignment_of_v<Chunk>));
#else
		auto chunk = reinterpret_cast<Chunk*>(aligned_alloc(sizeof(Chunk), std::alignment_of_v<Chunk>));
#endif // WIN32
		chunks.push_back(chunk);
	}
	return entityNum++;
}

tuple<void*, size_t> Archetype::At(CmptType type, size_t idx) const {
	assert(idx <= entityNum);

	auto size_target = type2size.find(type);
	if (size_target == type2size.end())
		return { nullptr, 0 };
	
	size_t size = Sizeof(type);
	size_t offset = Offsetof(type);
	size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();

	return { buffer + offset + idxInChunk * size,size };
}

tuple<vector<Entity*>, vector<vector<void*>>, vector<size_t>> Archetype::Locate(const std::set<CmptType>& cmptTypes) const {
	assert(types.IsContain(cmptTypes));
	vector<vector<void*>> chunkCmpts(chunks.size());
	vector<Entity*> chunkEntity;

	for (size_t i = 0; i < chunks.size(); i++) {
		auto data = chunks[i]->Data();
		for (const auto& type : cmptTypes)
			chunkCmpts[i].push_back(data + Offsetof(type));
		chunkEntity.push_back(reinterpret_cast<Entity*>(data + Offsetof(CmptType::Of<Entity>())));
	}

	vector<size_t> sizes;

	for (const auto& type : cmptTypes)
		sizes.push_back(Sizeof(type));


	return { chunkEntity, chunkCmpts, sizes };
}

size_t Archetype::Erase(size_t idx) {
	assert(idx < entityNum);

	size_t dstIdxInChunk = idx % chunkCapacity;
	byte* dstBuffer = chunks[idx / chunkCapacity]->Data();

	size_t movedIdx;
	
	if (idx != entityNum - 1) {
		movedIdx = entityNum - 1;

		size_t srcIdxInChunk = movedIdx % chunkCapacity;
		byte* srcBuffer = chunks[movedIdx / chunkCapacity]->Data();

		for (auto type : types) {
			auto size = Sizeof(type);
			auto offset = Offsetof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;
			RuntimeCmptTraits::Instance().Destruct(type, dst);
			RuntimeCmptTraits::Instance().MoveConstruct(type, size, dst, src);
		}
	}
	else {
		movedIdx = static_cast<size_t>(-1);
		for (auto type : types) {
			auto size = Sizeof(type);
			auto offset = Offsetof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			RuntimeCmptTraits::Instance().Destruct(type, dst);
		}
	}

	entityNum--;

	if (chunks.size() * chunkCapacity - entityNum >= chunkCapacity) {
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

	for (const auto& type : types) {
		if (type == CmptType::Of<Entity>())
			continue;
		auto [cmpt, size] = At(type, idx);
		rst.emplace_back(type, cmpt);
	}

	return rst;
}
