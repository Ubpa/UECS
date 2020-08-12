#include <UECS/detail/Archetype.h>

#include <UECS/EntityMngr.h>

using namespace Ubpa::UECS;
using namespace std;

Archetype::~Archetype() {
	for (const auto& type : types.data) {
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
		entityMngr->sharedChunkPool.Recycle(chunk);
}

void Archetype::SetLayout() {
	vector<size_t> alignments;
	vector<size_t> sizes;

	const size_t numType = types.data.size();

	alignments.reserve(numType);
	sizes.reserve(numType);
	type2offset.reserve(numType);

	for (const auto& type : types.data) {
		alignments.push_back(cmptTraits.Alignof(type));
		sizes.push_back(cmptTraits.Sizeof(type));
	}

	auto layout = Chunk::GenLayout(alignments, sizes);
	chunkCapacity = layout.capacity;

	size_t i = 0;
	for (const auto& type : types.data)
		type2offset.emplace(type, layout.offsets[i++]);
}

Archetype* Archetype::New(EntityMngr* entityMngr, const CmptType* types, size_t num) {
	assert(NotContainEntity(types, num));

	auto rst = new Archetype{ entityMngr };
	rst->types.Insert(types, num);
	rst->types.data.insert(CmptType::Of<Entity>);
	rst->cmptTraits.Register<Entity>();
	for (size_t i = 0; i < num; i++)
		rst->cmptTraits.Register(types[i]);
	rst->SetLayout();
	return rst;
}

Archetype* Archetype::Add(const Archetype* from, const CmptType* types, size_t num) {
	assert(NotContainEntity(types, num));
	assert(!from->types.ContainsAny(types, num));

	Archetype* rst = new Archetype{ from->entityMngr };

	rst->types = from->types;
	rst->cmptTraits = from->cmptTraits;
	rst->types.Insert(types, num);
	for (size_t i = 0; i < num; i++)
		rst->cmptTraits.Register(types[i]);

	rst->SetLayout();

	return rst;
}

Archetype* Archetype::Remove(const Archetype* from, const CmptType* types, size_t num) {
	assert(NotContainEntity(types, num));
	assert(from->types.ContainsAny(types, num));
	
	Archetype* rst = new Archetype{ from->entityMngr };

	rst->types = from->types;
	rst->cmptTraits = from->cmptTraits;
	rst->types.Erase(types, num);
	for (size_t i = 0; i < num; i++)
		rst->cmptTraits.Deregister(types[i]);

	rst->SetLayout();

	return rst;
}

size_t Archetype::Create(Entity e) {
	size_t idx = RequestBuffer();
	size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();

	const auto& rtdct = RTDCmptTraits::Instance();
	for (const auto& type : types.data) {
		if (type.Is<Entity>()) {
			constexpr size_t size = sizeof(Entity);
			size_t offset = Offsetof(type);
			memcpy(buffer + offset + idxInChunk * size, &e, size);
		}
		else {
			auto target = rtdct.default_constructors.find(type);
			if (target != rtdct.default_constructors.end()) {
				size_t size = cmptTraits.Sizeof(type);
				size_t offset = Offsetof(type);
				byte* dst = buffer + offset + idxInChunk * size;
				target->second(dst);
			}
		}
	}

	return idx;
}

size_t Archetype::RequestBuffer() {
	if (entityNum == chunks.size() * chunkCapacity) {
		auto chunk = entityMngr->sharedChunkPool.Request();
		chunks.push_back(chunk);
	}
	return entityNum++;
}

void* Archetype::At(CmptType type, size_t idx) const {
	assert(idx < entityNum);
	assert(types.Contains(type));
	
	size_t size = cmptTraits.Sizeof(type);
	size_t offset = Offsetof(type);
	size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();

	return buffer + offset + idxInChunk * size;
}

size_t Archetype::Instantiate(Entity e, size_t srcIdx) {
	assert(srcIdx < entityNum);

	size_t dstIdx = RequestBuffer();

	size_t srcIdxInChunk = srcIdx % chunkCapacity;
	byte* srcBuffer = chunks[srcIdx / chunkCapacity]->Data();
	size_t dstIdxInChunk = dstIdx % chunkCapacity;
	byte* dstBuffer = chunks[dstIdx / chunkCapacity]->Data();

	for (const auto& type : types.data) {
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

tuple<vector<Entity*>, vector<vector<CmptPtr>>, vector<size_t>> Archetype::Locate(const CmptLocator& locator) const {
	assert(types.IsMatch(locator));

	const size_t numChunk = chunks.size();
	const size_t numType = locator.CmptTypes().size();
	const size_t offsetEntity = Offsetof(CmptType::Of<Entity>);

	vector<vector<CmptPtr>> chunkCmpts(numChunk);
	vector<Entity*> chunkEntity(numChunk);

	for (size_t i = 0; i < numChunk; i++) {
		auto data = chunks[i]->Data();
		chunkCmpts[i].reserve(numType);
		for (const auto& type : locator.CmptTypes())
			chunkCmpts[i].emplace_back(type, data + Offsetof(type));
		chunkEntity[i] = reinterpret_cast<Entity*>(data + offsetEntity);
	}

	vector<size_t> sizes;
	sizes.reserve(numType);
	for (const auto& type : locator.CmptTypes())
		sizes.push_back(cmptTraits.Sizeof(type));

	return { chunkEntity, chunkCmpts, sizes };
}

void* Archetype::Locate(size_t chunkIdx, CmptType t) const {
	assert(types.Contains(t));
	assert(chunkIdx < chunks.size());
	auto buffer = chunks[chunkIdx]->Data();
	return buffer + Offsetof(t);
}

size_t Archetype::Erase(size_t idx) {
	assert(idx < entityNum);

	size_t dstIdxInChunk = idx % chunkCapacity;
	byte* dstBuffer = chunks[idx / chunkCapacity]->Data();

	size_t movedIdx;
	
	if (idx != entityNum - 1) {
		size_t movedIdxInArchetype = entityNum - 1;

		size_t srcIdxInChunk = movedIdxInArchetype % chunkCapacity;
		byte* srcBuffer = chunks[movedIdxInArchetype / chunkCapacity]->Data();

		for (const auto& type : types.data) {
			auto size = cmptTraits.Sizeof(type);
			auto offset = Offsetof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;

			if (type.Is<Entity>())
				movedIdx = reinterpret_cast<Entity*>(src)->Idx();

			cmptTraits.MoveAssign(type, dst, src);
			cmptTraits.Destruct(type, src);
		}
	}
	else {
		movedIdx = size_t_invalid;

		for (const auto& type : types.data) {
			auto size = cmptTraits.Sizeof(type);
			auto offset = Offsetof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			cmptTraits.Destruct(type, dst);
		}
	}

	entityNum--;

	if (chunks.size() * chunkCapacity - entityNum >= chunkCapacity) {
		Chunk* chunk = chunks.back();
		entityMngr->sharedChunkPool.Recycle(chunk);
		chunks.pop_back();
	}

	return movedIdx;
}

vector<CmptPtr> Archetype::Components(size_t idx) const {
	vector<CmptPtr> rst;

	for (const auto& type : types.data) {
		if (type.Is<Entity>())
			continue;
		rst.emplace_back(type, At(type, idx));
	}

	return rst;
}

size_t Archetype::EntityNumOfChunk(size_t chunkIdx) const noexcept {
	assert(chunkIdx < chunks.size());

	if (chunkIdx == chunks.size() - 1)
		return entityNum - (chunks.size() - 1) * chunkCapacity;
	else
		return chunkCapacity;
}

CmptTypeSet Archetype::GenCmptTypeSet(const CmptType* types, size_t num) {
	assert(NotContainEntity(types, num));
	CmptTypeSet typeset;
	typeset.Insert(types, num);
	typeset.data.insert(CmptType::Of<Entity>);
	return typeset;
}


bool Archetype::NotContainEntity(const CmptType* types, size_t num) noexcept {
	assert(types || num == 0);
	for (size_t i = 0; i < num; i++) {
		if (types[i].Is<Entity>())
			return false;
	}
	return true;
}
