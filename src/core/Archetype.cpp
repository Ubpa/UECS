#include "Archetype.hpp"

using namespace Ubpa::UECS;
using namespace std;

Archetype::~Archetype() {
	for (const auto& type : types.data) {
		auto trait = cmptTraits.GetTrait(type);
		if(!trait || !trait->dtor)
			continue;
		for (std::size_t k = 0; k < chunks.size(); k++) {
			std::size_t num = EntityNumOfChunk(k);
			byte* buffer = chunks[k]->Data();
			byte* beg = buffer + trait->offset;
			for (std::size_t i = 0; i < num; i++) {
				byte* address = beg + i * trait->size;
				trait->dtor(address);
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
	entityNum = src.entityNum;
	chunkCapacity = src.chunkCapacity;

	chunks.resize(src.chunks.size(), nullptr);
	for (std::size_t i = 0; i < src.chunks.size(); i++) {
		auto* srcChunk = src.chunks[i];
		auto* dstChunk = chunks[i] = chunkAllocator.allocate(1);
		std::size_t num = src.EntityNumOfChunk(i);
		for (auto type : types.data) {
			auto trait = cmptTraits.GetTrait(type);
			auto offset = trait->offset;
			auto* srcBegin = srcChunk->Data() + offset;
			auto* dstBegin = dstChunk->Data() + offset;
			if (trait && trait->copy_ctor) {
				for (std::size_t j = 0; j < num; j++) {
					auto offset_j = j * trait->size;
					trait->copy_ctor(dstBegin + offset_j, srcBegin + offset_j);
				}
			}
			else
				memcpy(dstBegin, srcBegin, num * trait->size);
		}
	}
}

void Archetype::SetLayout() {
	small_vector<std::size_t, 16> alignments;
	small_vector<std::size_t, 16> sizes;

	const std::size_t numType = types.data.size();

	alignments.reserve(numType);
	sizes.reserve(numType);

	for (const auto& trait : cmptTraits.GetTraits()) {
		alignments.push_back(trait.alignment);
		sizes.push_back(trait.size);
	}

	auto layout = Chunk::GenLayout(alignments, sizes);
	for (std::size_t i = 0; i < layout.offsets.size(); i++)
		cmptTraits.GetTraits()[i].offset = layout.offsets[i];
	chunkCapacity = layout.capacity;
}

Archetype* Archetype::New(RTDCmptTraits& rtdCmptTraits, std::pmr::polymorphic_allocator<Chunk> chunkAllocator, std::span<const TypeID> types) {
	assert(NotContainEntity(types));

	auto* rst = new Archetype{ chunkAllocator };
	rst->types.Insert(types);
	rst->types.data.insert(TypeID_of<Entity>);
	rst->cmptTraits.Register(rtdCmptTraits, TypeID_of<Entity>);

	for (const auto& type : types)
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
		auto trait = cmptTraits.GetTrait(type);
		const std::size_t offset = trait->offset;
		if (type.Is<Entity>()) {
			constexpr std::size_t size = sizeof(Entity);
			memcpy(buffer + offset + idxInChunk * size, &e, size);
		}
		else {
			auto target = rtdCmptTraits.GetDefaultConstructors().find(type);
			if (target == rtdCmptTraits.GetDefaultConstructors().end())
				continue;
			const auto& ctor = target->second;
			byte* dst = buffer + offset + idxInChunk * trait->size;
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
	
	auto trait = cmptTraits.GetTrait(type);
	std::size_t size = trait->size;
	std::size_t offset = trait->offset;
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
		auto trait = cmptTraits.GetTrait(type);
		std::size_t offset = trait->offset;

		if (type.Is<Entity>()) {
			constexpr std::size_t size = sizeof(Entity);
			memcpy(dstBuffer + offset + dstIdxInChunk * size, &e, size);
		}
		else {
			std::size_t size = trait->size;
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;

			trait->CopyConstruct(dst, src);
		}
	}

	return dstIdx;
}

std::tuple<
	Ubpa::small_vector<Entity*, 16>,
	Ubpa::small_vector<Ubpa::small_vector<CmptAccessPtr, 16>, 16>,
	Ubpa::small_vector<std::size_t, 16>
>
Archetype::Locate(const CmptLocator& locator) const {
	assert(types.IsMatch(locator));

	const std::size_t numChunk = chunks.size();
	const std::size_t numType = locator.AccessTypeIDs().size();
	const std::size_t offsetEntity = cmptTraits.GetTrait(TypeID_of<Entity>)->offset;

	Ubpa::small_vector<Ubpa::small_vector<CmptAccessPtr, 16>, 16> chunkCmpts(numChunk);
	Ubpa::small_vector<Entity*, 16> chunkEntity(numChunk);

	for (std::size_t i = 0; i < numChunk; i++) {
		byte* data = chunks[i]->Data();
		chunkCmpts[i].reserve(numType);
		for (const auto& type : locator.AccessTypeIDs())
			chunkCmpts[i].emplace_back(type, data + cmptTraits.GetTrait(type)->offset);
		chunkEntity[i] = reinterpret_cast<Entity*>(data + offsetEntity);
	}

	Ubpa::small_vector<std::size_t, 16> sizes;
	sizes.reserve(numType);
	for (const auto& type : locator.AccessTypeIDs())
		sizes.push_back(cmptTraits.GetTrait(type)->size);

	return { chunkEntity, chunkCmpts, sizes };
}

void* Archetype::Locate(std::size_t chunkIdx, TypeID t) const {
	assert(chunkIdx < chunks.size());
	if (!types.Contains(t))
		return nullptr;

	auto* buffer = chunks[chunkIdx]->Data();
	return buffer + cmptTraits.GetTrait(t)->offset;
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
			auto trait = cmptTraits.GetTrait(type);
			std::size_t size = trait->size;
			std::size_t offset = trait->offset;
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			byte* src = srcBuffer + offset + srcIdxInChunk * size;

			if (type.Is<Entity>())
				movedIdx = reinterpret_cast<Entity*>(src)->Idx();

			trait->MoveAssign(dst, src);
			trait->Destruct(src);
		}
	}
	else {
		for (const auto& type : types.data) {
			auto trait = cmptTraits.GetTrait(type);
			std::size_t size = trait->size;
			std::size_t offset = trait->offset;
			byte* dst = dstBuffer + offset + dstIdxInChunk * size;
			trait->Destruct(dst);
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
