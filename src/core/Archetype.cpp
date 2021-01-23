#include <UECS/details/Archetype.h>

using namespace Ubpa::UECS;
using namespace std;

Archetype::~Archetype() {
	for (const auto& type : types.data) {
		auto target = cmptTraits.destructors.find(type);
		if(target == cmptTraits.destructors.end())
			continue;
		const auto& destructor = target->second;
		std::size_t size = cmptTraits.Sizeof(type);
		std::size_t offset = Offsetof(type);
		for (std::size_t k = 0; k < chunks.size(); k++) {
			std::size_t num = EntityNumOfChunk(k);
			byte* buffer = chunks[k]->Data();
			byte* beg = buffer + offset;
			for (std::size_t i = 0; i < num; i++) {
				byte* address = beg + i * size;
				destructor(address);
			}
		}
	}
	// FastClear by EntityMngr
	//for (Chunk* chunk : chunks)
	//	entityMngr->sharedChunkPool.Recycle(chunk);
}

Archetype::Archetype(std::pmr::polymorphic_allocator<Chunk> chunkAllocator, const Archetype& src)
	: chunkAllocator{ chunkAllocator }
{
	types = src.types;
	cmptTraits = src.cmptTraits;
	type2offset = src.type2offset;
	entityNum = src.entityNum;
	chunkCapacity = src.chunkCapacity;

	chunks.resize(src.chunks.size(), nullptr);
	for (std::size_t i = 0; i < src.chunks.size(); i++) {
		auto* srcChunk = src.chunks[i];
		auto* dstChunk = chunks[i] = chunkAllocator.allocate(1);
		std::size_t num = src.EntityNumOfChunk(i);
		for (auto type : types.data) {
			auto offset = Offsetof(type);
			auto* srcBegin = srcChunk->Data() + offset;
			auto* dstBegin = dstChunk->Data() + offset;
			auto size = cmptTraits.Sizeof(type);
			auto target = cmptTraits.copy_constructors.find(type);
			if (target != cmptTraits.copy_constructors.end()) {
				const auto& copy_ctor = target->second;
				for (std::size_t j = 0; j < num; j++) {
					auto offset_j = j * size;
					copy_ctor(dstBegin + offset_j, srcBegin + offset_j);
				}
			}
			else
				memcpy(dstBegin, srcBegin, num * size);
		}
	}
}

void Archetype::SetLayout() {
	vector<std::size_t> alignments;
	vector<std::size_t> sizes;

	const std::size_t numType = types.data.size();

	alignments.reserve(numType);
	sizes.reserve(numType);
	type2offset.reserve(numType);

	for (const auto& type : types.data) {
		alignments.push_back(cmptTraits.Alignof(type));
		sizes.push_back(cmptTraits.Sizeof(type));
	}

	auto layout = Chunk::GenLayout(alignments, sizes);
	chunkCapacity = layout.capacity;

	std::size_t i = 0;
	for (const auto& type : types.data)
		type2offset.emplace(type, layout.offsets[i++]);
}

Archetype* Archetype::New(RTDCmptTraits& rtdCmptTraits, std::pmr::polymorphic_allocator<Chunk> chunkAllocator, std::span<const TypeID> types) {
	assert(NotContainEntity(types));

	auto* rst = new Archetype{ chunkAllocator };
	rst->types.Insert(types);
	rst->types.data.insert(TypeID_of<Entity>);
	rst->cmptTraits.Register<Entity>();
	for(const auto& type : types)
		rst->cmptTraits.Register(rtdCmptTraits, type);
	rst->SetLayout();
	return rst;
}

Archetype* Archetype::Add(RTDCmptTraits& rtdCmptTraits, const Archetype* from, std::span<const TypeID> types) {
	assert(NotContainEntity(types));
	assert(!from->types.ContainsAll(types));

	auto* rst = new Archetype{ from->chunkAllocator };

	rst->types = from->types;
	rst->cmptTraits = from->cmptTraits;
	rst->types.Insert(types);
	for (const auto& type : types)
		rst->cmptTraits.Register(rtdCmptTraits, type);

	rst->SetLayout();

	return rst;
}

Archetype* Archetype::Remove(const Archetype* from, std::span<const TypeID> types) {
	assert(NotContainEntity(types));
	assert(from->types.ContainsAny(types));
	
	auto* rst = new Archetype{ from->chunkAllocator };

	rst->types = from->types;
	rst->cmptTraits = from->cmptTraits;
	rst->types.Erase(types);
	for (const auto& type : types)
		rst->cmptTraits.Deregister(type);

	rst->SetLayout();

	return rst;
}

std::size_t Archetype::Create(RTDCmptTraits& rtdCmptTraits, Entity e) {
	std::size_t idx = RequestBuffer();
	std::size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();

	for (const auto& type : types.data) {
		if (type.Is<Entity>()) {
			constexpr std::size_t size = sizeof(Entity);
			std::size_t offset = Offsetof(type);
			memcpy(buffer + offset + idxInChunk * size, &e, size);
		}
		else {
			auto target = rtdCmptTraits.GetDefaultConstructors().find(type);
			if (target == rtdCmptTraits.GetDefaultConstructors().end())
				continue;
			const auto& ctor = target->second;
			std::size_t size = cmptTraits.Sizeof(type);
			std::size_t offset = Offsetof(type);
			byte* dst = buffer + offset + idxInChunk * size;
			ctor(dst);
		}
	}

	return idx;
}

std::size_t Archetype::RequestBuffer() {
	if (entityNum == chunks.size() * chunkCapacity) {
		auto* chunk = chunkAllocator.allocate(1);
		chunks.push_back(chunk);
	}
	return entityNum++;
}

void* Archetype::At(TypeID type, std::size_t idx) const {
	assert(idx < entityNum);
	
	if (!types.Contains(type))
		return nullptr;
	
	std::size_t size = cmptTraits.Sizeof(type);
	std::size_t offset = Offsetof(type);
	std::size_t idxInChunk = idx % chunkCapacity;
	byte* buffer = chunks[idx / chunkCapacity]->Data();

	return buffer + offset + idxInChunk * size;
}

std::size_t Archetype::Instantiate(Entity e, std::size_t srcIdx) {
	assert(srcIdx < entityNum);

	std::size_t dstIdx = RequestBuffer();

	std::size_t srcIdxInChunk = srcIdx % chunkCapacity;
	std::size_t dstIdxInChunk = dstIdx % chunkCapacity;
	byte* srcBuffer = chunks[srcIdx / chunkCapacity]->Data();
	byte* dstBuffer = chunks[dstIdx / chunkCapacity]->Data();

	for (const auto& type : types.data) {
		std::size_t offset = Offsetof(type);

		if (type.Is<Entity>()) {
			constexpr std::size_t size = sizeof(Entity);
			memcpy(dstBuffer + offset + dstIdxInChunk * size, &e, size);
		}
		else {
			std::size_t size = cmptTraits.Sizeof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;

			cmptTraits.CopyConstruct(type, dst, src);
		}
	}

	return dstIdx;
}

tuple<vector<Entity*>, vector<vector<CmptAccessPtr>>, vector<std::size_t>> Archetype::Locate(const CmptLocator& locator) const {
	assert(types.IsMatch(locator));

	const std::size_t numChunk = chunks.size();
	const std::size_t numType = locator.AccessTypeIDs().size();
	const std::size_t offsetEntity = Offsetof(TypeID_of<Entity>);

	vector<vector<CmptAccessPtr>> chunkCmpts(numChunk);
	vector<Entity*> chunkEntity(numChunk);

	for (std::size_t i = 0; i < numChunk; i++) {
		byte* data = chunks[i]->Data();
		chunkCmpts[i].reserve(numType);
		for (const auto& type : locator.AccessTypeIDs())
			chunkCmpts[i].emplace_back(type, data + Offsetof(type));
		chunkEntity[i] = reinterpret_cast<Entity*>(data + offsetEntity);
	}

	vector<std::size_t> sizes;
	sizes.reserve(numType);
	for (const auto& type : locator.AccessTypeIDs())
		sizes.push_back(cmptTraits.Sizeof(type));

	return { chunkEntity, chunkCmpts, sizes };
}

void* Archetype::Locate(std::size_t chunkIdx, TypeID t) const {
	assert(chunkIdx < chunks.size());
	if (!types.Contains(t))
		return nullptr;

	auto* buffer = chunks[chunkIdx]->Data();
	return buffer + Offsetof(t);
}

std::size_t Archetype::Erase(std::size_t idx) {
	assert(idx < entityNum);

	std::size_t dstIdxInChunk = idx % chunkCapacity;
	byte* dstBuffer = chunks[idx / chunkCapacity]->Data();

	std::size_t movedIdx = static_cast<std::size_t>(-1);
	
	if (idx != entityNum - 1) {
		std::size_t movedIdxInArchetype = entityNum - 1;

		std::size_t srcIdxInChunk = movedIdxInArchetype % chunkCapacity;
		byte* srcBuffer = chunks[movedIdxInArchetype / chunkCapacity]->Data();

		for (const auto& type : types.data) {
			std::size_t size = cmptTraits.Sizeof(type);
			std::size_t offset = Offsetof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;

			if (type.Is<Entity>())
				movedIdx = reinterpret_cast<Entity*>(src)->Idx();

			cmptTraits.MoveAssign(type, dst, src);
			cmptTraits.Destruct(type, src);
		}
	}
	else {
		for (const auto& type : types.data) {
			std::size_t size = cmptTraits.Sizeof(type);
			std::size_t offset = Offsetof(type);
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			cmptTraits.Destruct(type, dst);
		}
	}

	entityNum--;

	if (chunks.size() * chunkCapacity - entityNum >= chunkCapacity) {
		Chunk* chunk = chunks.back();
		chunkAllocator.deallocate(chunk, 1);
		chunks.pop_back();
	}

	return movedIdx;
}

vector<CmptPtr> Archetype::Components(std::size_t idx) const {
	vector<CmptPtr> rst;

	for (const auto& type : types.data) {
		if (type.Is<Entity>())
			continue;
		rst.emplace_back(type, At(type, idx));
	}

	return rst;
}

std::size_t Archetype::EntityNumOfChunk(std::size_t chunkIdx) const noexcept {
	assert(chunkIdx < chunks.size());

	if (chunkIdx == chunks.size() - 1)
		return entityNum - (chunks.size() - 1) * chunkCapacity;
	else
		return chunkCapacity;
}

TypeIDSet Archetype::GenTypeIDSet(std::span<const TypeID> types) {
	assert(NotContainEntity(types));
	TypeIDSet typeset;
	typeset.Insert(types);
	typeset.data.insert(TypeID_of<Entity>);
	return typeset;
}

bool Archetype::NotContainEntity(std::span<const TypeID> types) noexcept {
	for (const auto& type : types) {
		if (type.Is<Entity>())
			return false;
	}
	return true;
}
