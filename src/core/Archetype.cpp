#include "Archetype.hpp"

using namespace Ubpa::UECS;
using namespace std;

Archetype::Archetype(std::pmr::memory_resource* rsrc, std::pmr::memory_resource* world_rsrc, std::uint64_t version) noexcept :
	chunkAllocator{ rsrc },
	version{ version },
	world_rsrc{ world_rsrc }{}

Archetype::~Archetype() {
	if (cmptTraits.IsTrivial())
		return;

	for (std::size_t i = 0; i < cmptTraits.GetTraits().size(); i++) {
		const auto& trait = cmptTraits.GetTraits()[i];
		if (!trait.dtor)
			continue;
		for (std::size_t k = 0; k < chunks.size(); k++) {
			std::size_t num = EntityNumOfChunk(k);
			std::uint8_t* buffer = chunks[k]->data;
			std::uint8_t* beg = buffer + offsets[i];
			for (std::size_t i = 0; i < num; i++) {
				std::uint8_t* address = beg + i * trait.size;
				trait.dtor(address);
			}
		}
	}
	// FastClear by EntityMngr
	//for (Chunk* chunk : chunks)
	//	entityMngr->sharedChunkPool.Recycle(chunk);
}

Archetype::Archetype(std::pmr::memory_resource* rsrc, std::pmr::memory_resource* world_rsrc, const Archetype& src) :
	chunkAllocator{ rsrc },
	world_rsrc{ world_rsrc }
{
	cmptTraits = src.cmptTraits;
	entityNum = src.entityNum;
	chunkCapacity = src.chunkCapacity;
	offsets = src.offsets;
	version = src.version;

	chunks.resize(src.chunks.size(), nullptr);

	if (cmptTraits.IsTrivial()) {
		// [0, src.chunks.size() - 1)
		for (std::size_t i = 0; i < src.chunks.size() - 1; i++) {
			chunks[i] = chunkAllocator.allocate(1);
			auto* srcChunk = src.chunks[i];
			auto* dstChunk = chunks[i];
			std::memcpy(dstChunk->data, srcChunk->data, sizeof(Chunk));
			dstChunk->GetHead()->UpdateVersion(version);
			dstChunk->GetHead()->archetype = this;
		}
		{ // src.chunks.size() - 1
			auto* srcChunk = src.chunks.back();
			auto* dstChunk = chunks.back() = chunkAllocator.allocate(1);
			std::memcpy(dstChunk, srcChunk, sizeof(Chunk::Head) + sizeof(Chunk::Head::CmptInfo) * cmptTraits.GetTypes().size());
			dstChunk->GetHead()->UpdateVersion(version);
			dstChunk->GetHead()->archetype = this;
			std::size_t num = src.EntityNumOfChunk(src.chunks.size() - 1);
			for (std::size_t i = 0; i < cmptTraits.GetTraits().size(); i++) {
				const auto& trait = cmptTraits.GetTraits()[i];
				auto* srcBegin = srcChunk->data + offsets[i];
				auto* dstBegin = dstChunk->data + offsets[i];
				if (trait.copy_ctor) {
					for (std::size_t j = 0; j < num; j++) {
						auto offset_j = j * trait.size;
						trait.copy_ctor(dstBegin + offset_j, srcBegin + offset_j, world_rsrc);
					}
				}
				else
					memcpy(dstBegin, srcBegin, num * trait.size);
			}
		}
	}
	else {
		for (std::size_t i = 0; i < src.chunks.size(); i++) {
			auto* srcChunk = src.chunks[i];
			auto* dstChunk = chunks[i] = chunkAllocator.allocate(1);
			std::memcpy(dstChunk, srcChunk, sizeof(Chunk::Head) + sizeof(Chunk::Head::CmptInfo) * cmptTraits.GetTypes().size());
			dstChunk->GetHead()->UpdateVersion(version);
			dstChunk->GetHead()->archetype = this;
			std::size_t num = src.EntityNumOfChunk(i);
			for (std::size_t j = 0; j < cmptTraits.GetTraits().size(); j++) {
				const auto& trait = cmptTraits.GetTraits()[j];
				auto* cursor_src = srcChunk->data + offsets[j];
				auto* cursor_dst = dstChunk->data + offsets[j];
				if (trait.copy_ctor) {
					for (std::size_t k = 0; k < num; k++) {
						trait.copy_ctor(cursor_dst, cursor_src, world_rsrc);
						cursor_src += trait.size;
						cursor_dst += trait.size;
					}
				}
				else
					memcpy(cursor_dst, cursor_src, num * trait.size);
			}
		}
	}
}

void Archetype::SetLayout() {
	struct Item {
		std::size_t alignment;
		std::size_t idx;
	};

	const auto& traits = cmptTraits.GetTraits();
	small_vector<Item> items;
	items.resize(traits.size());

	std::size_t sum_size = 0;
	for (std::size_t i = 0; i < items.size(); i++) {
		items[i].idx = i;
		const auto& trait = cmptTraits.GetTraits()[i];
		items[i].alignment = trait.alignment;
		sum_size += trait.size;
	}
	std::sort(items.begin(), items.end(), [](const Item& lhs, const Item& rhs) {
		return lhs.alignment > rhs.alignment;
	});
	sum_size = (sum_size + items.front().alignment - 1) & ~(items.front().alignment - 1);

	std::size_t cmptInfosSize = sizeof(Chunk::Head::CmptInfo) * cmptTraits.GetTypes().size();
	chunkCapacity = (ChunkSize - sizeof(Chunk::Head) - cmptInfosSize) / sum_size;
	offsets.resize(traits.size());
	std::size_t offset = sizeof(Chunk::Head) + cmptInfosSize;
	offset = (offset + items.front().alignment - 1) & ~(items.front().alignment - 1);
	for (std::size_t i = 0; i < items.size(); i++) {
		const auto& idx = items[i].idx;
		offsets[idx] = offset;
		offset += chunkCapacity * traits[idx].size;
	}
}

Archetype* Archetype::New(
	CmptTraits& rtdCmptTraits,
	std::pmr::memory_resource* rsrc,
	std::pmr::memory_resource* world_rsrc,
	std::span<const TypeID> types,
	std::uint64_t version)
{
	assert(std::find(types.begin(), types.end(), TypeID_of<Entity>) == types.end());

	auto* rst = new Archetype{ rsrc, world_rsrc, version };

	rst->cmptTraits.Register(rtdCmptTraits, TypeID_of<Entity>);
	for (const auto& type : types)
		rst->cmptTraits.Register(rtdCmptTraits, type);

	rst->SetLayout();

	return rst;
}

Archetype* Archetype::Add(CmptTraits& rtdCmptTraits, const Archetype* from, std::span<const TypeID> types) {
	assert(std::find(types.begin(), types.end(), TypeID_of<Entity>) == types.end());
	assert(std::find_if_not(types.begin(), types.end(), [&](const auto& type) { return from->cmptTraits.GetTypes().contains(type); }) != types.end());

	auto* rst = new Archetype{ from->chunkAllocator.resource(), from->world_rsrc, from->version };

	rst->cmptTraits = from->cmptTraits;
	for (const auto& type : types)
		rst->cmptTraits.Register(rtdCmptTraits, type);

	rst->SetLayout();

	return rst;
}

Archetype* Archetype::Remove(const Archetype* from, std::span<const TypeID> types) {
	assert(std::find(types.begin(), types.end(), TypeID_of<Entity>) == types.end());
	assert(std::find_if(types.begin(), types.end(), [&](const auto& type) { return from->cmptTraits.GetTypes().contains(type); }) != types.end());
	
	auto* rst = new Archetype{ from->chunkAllocator.resource(), from->world_rsrc, from->version };

	rst->cmptTraits = from->cmptTraits;

	for (const auto& type : types)
		rst->cmptTraits.Deregister(type);

	rst->SetLayout();

	return rst;
}

std::size_t Archetype::Create(Entity e) {
	std::size_t idx = RequestBuffer();
	std::size_t idxInChunk = idx % chunkCapacity;
	Chunk* chunk = chunks[idx / chunkCapacity];
	std::uint8_t* buffer = chunk->data;

	for (std::size_t i = 0; i < cmptTraits.GetTraits().size(); i++) {
		const auto& trait = cmptTraits.GetTraits()[i];
		const std::size_t offset = offsets[i];
		const auto& type = *(cmptTraits.GetTypes().begin() + i);
		if (type.Is<Entity>()) {
			constexpr std::size_t size = sizeof(Entity);
			memcpy(buffer + offset + idxInChunk * size, &e, size);
		}
		else {
			std::uint8_t* dst = buffer + offset + idxInChunk * trait.size;
			trait.DefaultConstruct(dst, world_rsrc);
		}
	}
	chunk->GetHead()->UpdateVersion(version);

	return idx;
}

std::size_t Archetype::RequestBuffer() {
	if (entityNum == chunks.size() * chunkCapacity) {
		auto* chunk = chunkAllocator.allocate(1);

		// init chunk
		chunk->GetHead()->archetype = this;
		chunk->GetHead()->capacity = chunkCapacity;
		chunk->GetHead()->num_entity = 0;
		chunk->GetHead()->num_component = cmptTraits.GetTypes().size();
		for (std::size_t i = 0; i < cmptTraits.GetTypes().size(); i++) {
			auto& info = chunk->GetHead()->GetCmptInfos()[i];
			info.ID = cmptTraits.GetTypes().data()[i];
			info.offset = offsets[i];
		}
		chunk->GetHead()->UpdateVersion(version);
		chunks.push_back(chunk);
	}
	return entityNum++;
}

CmptAccessPtr Archetype::At(AccessTypeID type, std::size_t idx) const {
	assert(idx < entityNum);
	auto target = cmptTraits.GetTypes().find(type);
	if (target == cmptTraits.GetTypes().end())
		return nullptr;
	auto typeIdx = static_cast<std::size_t>(std::distance(cmptTraits.GetTypes().begin(), target));
	
	const auto& trait = cmptTraits.GetTraits()[typeIdx];
	std::size_t size = trait.size;
	std::size_t offset = offsets[typeIdx];
	std::size_t idxInChunk = idx % chunkCapacity;
	Chunk* chunk = chunks[idx / chunkCapacity];
	std::uint8_t* buffer = chunk->data;
	if(type.GetAccessMode() == AccessMode::WRITE)
		chunk->GetHead()->GetCmptInfos()[typeIdx].version = version;

	return { type, buffer + offset + idxInChunk * size };
}

std::size_t Archetype::Instantiate(Entity e, std::size_t srcIdx) {
	assert(srcIdx < entityNum);

	std::size_t dstIdx = RequestBuffer();

	std::size_t srcIdxInChunk = srcIdx % chunkCapacity;
	std::size_t dstIdxInChunk = dstIdx % chunkCapacity;
	std::uint8_t* srcBuffer = chunks[srcIdx / chunkCapacity]->data;
	Chunk* dstChunk = chunks[dstIdx / chunkCapacity];
	std::uint8_t* dstBuffer = dstChunk->data;

	for (std::size_t i = 0; i < cmptTraits.GetTypes().size(); i++) {
		const auto& trait = cmptTraits.GetTraits()[i];
		std::size_t offset = offsets[i];

		if (cmptTraits.GetTypes().data()[i].Is<Entity>()) {
			constexpr std::size_t size = sizeof(Entity);
			memcpy(dstBuffer + offset + dstIdxInChunk * size, &e, size);
		}
		else {
			std::size_t size = trait.size;
			std::uint8_t* dst = dstBuffer + offset + dstIdxInChunk * size;
			std::uint8_t* src = srcBuffer + offset + srcIdxInChunk * size;

			trait.CopyConstruct(dst, src, world_rsrc);
		}
	}

	dstChunk->GetHead()->UpdateVersion(version);

	return dstIdx;
}

std::tuple<
	Ubpa::small_vector<Entity*>,
	Ubpa::small_vector<Ubpa::small_vector<CmptAccessPtr>>,
	Ubpa::small_vector<std::size_t>
>
Archetype::Locate(std::span<const AccessTypeID> cmpts) const {
	assert(std::find_if_not(cmpts.begin(), cmpts.end(), [this](const TypeID& type) { return cmptTraits.GetTypes().contains(type); }) == cmpts.end());

	const std::size_t numChunk = chunks.size();
	const std::size_t numType = cmpts.size();
	const std::size_t entityIdx = static_cast<std::size_t>(std::distance(cmptTraits.GetTypes().begin(), cmptTraits.GetTypes().find(TypeID_of<Entity>)));
	const std::size_t offsetEntity = offsets[entityIdx];

	Ubpa::small_vector<Ubpa::small_vector<CmptAccessPtr>> chunkCmpts(numChunk);
	Ubpa::small_vector<Entity*> chunkEntity(numChunk);

	for (std::size_t i = 0; i < numChunk; i++) {
		Chunk* chunk = chunks[i];
		std::uint8_t* data = chunk->data;
		chunkCmpts[i].reserve(numType);
		for (const auto& type : cmpts) {
			const std::size_t idx = static_cast<std::size_t>(std::distance(cmptTraits.GetTypes().begin(), cmptTraits.GetTypes().find(type)));
			const std::size_t offset = offsets[idx];
			chunkCmpts[i].emplace_back(type, data + offset);
			if (type.GetAccessMode() == AccessMode::WRITE)
				chunk->GetHead()->GetCmptInfos()[idx].version = version;
		}
		chunkEntity[i] = reinterpret_cast<Entity*>(data + offsetEntity);
	}

	Ubpa::small_vector<std::size_t> sizes;
	sizes.reserve(numType);
	for (const auto& type : cmpts)
		sizes.push_back(cmptTraits.GetTrait(type).size);

	return { chunkEntity, chunkCmpts, sizes };
}

std::size_t Archetype::Erase(std::size_t idx) {
	assert(idx < entityNum);

	std::size_t dstIdxInChunk = idx % chunkCapacity;
	Chunk* dstChunk = chunks[idx / chunkCapacity];
	std::uint8_t* dstBuffer = dstChunk->data;

	std::size_t movedIdx = static_cast<std::size_t>(-1);
	
	if (idx != entityNum - 1) {
		std::size_t movedIdxInArchetype = entityNum - 1;

		std::size_t srcIdxInChunk = movedIdxInArchetype % chunkCapacity;
		Chunk* srcChunk = chunks[movedIdxInArchetype / chunkCapacity];
		std::uint8_t* srcBuffer = srcChunk->data;

		for (std::size_t i = 0; i < cmptTraits.GetTypes().size(); i++) {
			const auto& trait = cmptTraits.GetTraits()[i];
			std::size_t size = trait.size;
			std::size_t offset = offsets[i];
			std::uint8_t* dst = dstBuffer + offset + dstIdxInChunk * size;
			std::uint8_t* src = srcBuffer + offset + srcIdxInChunk * size;

			if (cmptTraits.GetTypes().data()[i].Is<Entity>())
				movedIdx = reinterpret_cast<Entity*>(src)->index;

			trait.MoveAssign(dst, src);
			trait.Destruct(src);
		}

		srcChunk->GetHead()->UpdateVersion(version);
	}
	else if(!cmptTraits.IsTrivial()) {
		for (std::size_t i = 0; i < cmptTraits.GetTypes().size(); i++) {
			const auto& trait = cmptTraits.GetTraits()[i];
			std::size_t size = trait.size;
			std::size_t offset = offsets[i];
			std::uint8_t* dst = dstBuffer + offset + dstIdxInChunk * size;
			trait.Destruct(dst);
		}
	}

	dstChunk->GetHead()->UpdateVersion(version);

	entityNum--;

	if (chunks.size() * chunkCapacity - entityNum >= chunkCapacity) {
		Chunk* chunk = chunks.back();
		chunkAllocator.deallocate(chunk, 1);
		chunks.pop_back();
	}

	return movedIdx;
}

vector<CmptAccessPtr> Archetype::Components(std::size_t idx, AccessMode mode) const {
	vector<CmptAccessPtr> rst;

	for (const auto& type : cmptTraits.GetTypes()) {
		if (type.Is<Entity>())
			continue;
		rst.push_back(At({ type,mode }, idx));
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

Ubpa::small_flat_set<Ubpa::TypeID> Archetype::GenTypeIDSet(std::span<const TypeID> types) {
	small_flat_set<TypeID> sorted_types(types.begin(), types.end());
	sorted_types.insert(TypeID_of<Entity>);
	return sorted_types;
}
