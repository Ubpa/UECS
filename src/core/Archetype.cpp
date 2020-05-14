#include <UECS/detail/Archetype.h>

#include <cstdlib>

using namespace std;
using namespace Ubpa;

void Archetype::SetLayout() {
	std::vector<size_t> alignments;
	std::vector<size_t> sizes;

	for (const auto& type : types) {
		alignments.push_back(cmptTraits.Alignof(type));
		sizes.push_back(cmptTraits.Sizeof(type));
	}

	auto layout = Chunk::GenLayout(alignments, sizes);
	chunkCapacity = layout.capacity;

	size_t i = 0;
	for (const auto& type : types)
		type2offset[type] = layout.offsets[i++];
}

Archetype::~Archetype() {
	for (const auto& type : types) {
		size_t size = cmptTraits.Sizeof(type);
		size_t offset = Offsetof(type);
		for (size_t i = 0; i < entityNum; i++) {
			size_t idxInChunk = i % chunkCapacity;
			byte* buffer = chunks[i / chunkCapacity]->Data();
			byte* address = buffer + offset + idxInChunk * size;
			cmptTraits.Destruct(type, address);
		}
	}
	for (Chunk* chunk : chunks)
		sharedChunkPool.Recycle(chunk);
}

size_t Archetype::RequestBuffer() {
	if (entityNum == chunks.size() * chunkCapacity) {
		auto chunk = sharedChunkPool.Request();
		chunks.push_back(chunk);
	}
	return entityNum++;
}

tuple<void*, size_t> Archetype::At(CmptType type, size_t idx) const {
	assert(idx < entityNum);
	assert(types.IsContain(type));
	
	size_t size = cmptTraits.Sizeof(type);
	size_t offset = Offsetof(type);
	size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();

	return { buffer + offset + idxInChunk * size,size };
}

size_t Archetype::Instantiate(Entity e, size_t srcIdx) {
	assert(srcIdx < entityNum);
	size_t dstIdx = RequestBuffer();

	size_t srcIdxInChunk = srcIdx % chunkCapacity;
	byte* srcBuffer = chunks[srcIdx / chunkCapacity]->Data();
	size_t dstIdxInChunk = dstIdx % chunkCapacity;
	byte* dstBuffer = chunks[dstIdx / chunkCapacity]->Data();

	for (auto type : types) {
		size_t offset = Offsetof(type);

		if (type.Is<Entity>()) {
			constexpr size_t size = sizeof(Entity);
			memcpy(dstBuffer + offset + dstIdxInChunk * size, &e, size);
		}
		else {
			size_t size = cmptTraits.Sizeof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;

			cmptTraits.CopyConstruct(type, dst, src);
		}
	}

	return dstIdx;
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
		sizes.push_back(cmptTraits.Sizeof(type));


	return { chunkEntity, chunkCmpts, sizes };
}

size_t Archetype::Erase(size_t idx) {
	assert(idx < entityNum);

	size_t dstIdxInChunk = idx % chunkCapacity;
	byte* dstBuffer = chunks[idx / chunkCapacity]->Data();

	size_t movedIdx = size_t_invalid;
	
	if (idx != entityNum - 1) {
		size_t movedIdxInArchetype = entityNum - 1;

		size_t srcIdxInChunk = movedIdxInArchetype % chunkCapacity;
		byte* srcBuffer = chunks[movedIdxInArchetype / chunkCapacity]->Data();

		for (auto type : types) {
			auto size = cmptTraits.Sizeof(type);
			auto offset = Offsetof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;

			if (type.Is<Entity>())
				movedIdx = reinterpret_cast<Entity*>(dst)->Idx();

			cmptTraits.Destruct(type, dst);
			cmptTraits.MoveConstruct(type, dst, src);
		}
	}
	else {
		for (auto type : types) {
			auto size = cmptTraits.Sizeof(type);
			auto offset = Offsetof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			cmptTraits.Destruct(type, dst);
		}
	}

	entityNum--;

	if (chunks.size() * chunkCapacity - entityNum >= chunkCapacity) {
		Chunk* chunk = chunks.back();
		sharedChunkPool.Recycle(chunk);
		chunks.pop_back();
	}

	return movedIdx;
}

vector<CmptPtr> Archetype::Components(size_t idx) const {
	vector<CmptPtr> rst;

	for (const auto& type : types) {
		if (type.Is<Entity>())
			continue;
		auto [cmpt, size] = At(type, idx);
		rst.emplace_back(type, cmpt);
	}

	return rst;
}
