#include <UECS/Chunk.hpp>

#include "Archetype.hpp"

#include <algorithm>

using namespace Ubpa::UECS;

bool Chunk::Contains(TypeID type) const noexcept {
	const auto infos = GetHead()->GetCmptInfos();
	auto target = std::lower_bound(infos.begin(), infos.end(), type, std::less<>{});
	return target != infos.end() && target->ID == type;
}

bool Chunk::DidChange(TypeID cmptType, std::size_t version) const noexcept {
	return GetComponentVersion(cmptType) >= version;
}

bool Chunk::DidOrderChange(std::size_t version) const noexcept {
	return GetHead()->order_version >= version;
}

std::uint64_t Chunk::GetComponentVersion(TypeID cmptType) const noexcept {
	assert(Contains(cmptType));
	const auto infos = GetHead()->GetCmptInfos();
	auto target = std::lower_bound(infos.begin(), infos.end(), cmptType, std::less<>{});
	return target->version;
}

void* Chunk::GetCmptArray(TypeID cmptType) const noexcept {
	const auto infos = GetHead()->GetCmptInfos();
	auto target = std::lower_bound(infos.begin(), infos.end(), cmptType, std::less<>{});
	if (target == infos.end() || target->ID != cmptType)
		return nullptr;

	return (std::uint8_t*)data + target->offset;
}

bool Chunk::HasAnyChange(std::span<const TypeID> types, std::uint64_t version) const noexcept {
	const auto infos = GetHead()->GetCmptInfos();
	for (const auto& type : types) {
		auto target = std::lower_bound(infos.begin(), infos.end(), type, std::less<>{});
		if (target == infos.end())
			continue;

		if (target->version >= version)
			return true;
	}

	return false;
}

std::size_t Chunk::Erase(std::size_t idx) {
	auto* head = GetHead();
	auto& entityNum = head->num_entity;
	assert(idx < entityNum);
	const auto& cmptTraits = head->archetype->GetCmptTraits();

	std::size_t dstIdxInChunk = idx;
	Chunk* dstChunk = this;
	std::uint8_t* dstBuffer = data;

	std::size_t movedEntityIdx;

	if (idx != entityNum - 1) {
		std::size_t srcIdxInChunk = entityNum - 1;

		auto entityTypeIdx = static_cast<std::size_t>(std::distance(cmptTraits.GetTypes().begin(), cmptTraits.GetTypes().find(TypeID_of<Entity>)));
		movedEntityIdx = reinterpret_cast<Entity*>(head->GetCmptInfos()[entityTypeIdx].offset + sizeof(Entity) * srcIdxInChunk)->index;

		for (std::size_t i = 0; i < head->num_component; i++) {
			const auto& trait = cmptTraits.GetTraits()[i];
			std::size_t size = trait.size;
			std::size_t offset = head->GetCmptInfos()[i].offset;
			std::uint8_t* dst = data + offset + dstIdxInChunk * size;
			std::uint8_t* src = data + offset + srcIdxInChunk * size;

			trait.MoveAssign(dst, src);
			trait.Destruct(src);
		}
	}
	else {
		movedEntityIdx = static_cast<std::size_t>(-1);

		if (!cmptTraits.IsTrivial()) {
			for (std::size_t i = 0; i < cmptTraits.GetTypes().size(); i++) {
				const auto& trait = cmptTraits.GetTraits()[i];
				std::size_t size = trait.size;
				std::size_t offset = head->GetCmptInfos()[i].offset;
				std::uint8_t* dst = dstBuffer + offset + dstIdxInChunk * size;
				trait.Destruct(dst);
			}
		}
	}

	head->UpdateVersion(head->archetype->Version());

	entityNum--;

	return movedEntityIdx;
}

void Chunk::ApplyChanges(std::span<const AccessTypeID> types) {
	const auto infos = GetHead()->GetCmptInfos();
	for (const auto& type : types) {
		if (type.GetAccessMode() != AccessMode::WRITE)
			continue;

		auto target = std::lower_bound(infos.begin(), infos.end(), type, std::less<>{});
		if (target == infos.end() || target->ID != type)
			continue;

		auto idx = static_cast<std::size_t>(std::distance(infos.begin(), target));
		infos[idx].version = GetHead()->archetype->Version();
	}
}

std::tuple<Entity*, Ubpa::small_vector<CmptAccessPtr>, Ubpa::small_vector<std::size_t>> Chunk::Locate(std::span<const AccessTypeID> types) {
	const auto& cmptTraits = GetHead()->archetype->GetCmptTraits();
	const auto infos = GetHead()->GetCmptInfos();
	small_vector<CmptAccessPtr> cmpts;
	small_vector<std::size_t> sizes;
	cmpts.reserve(types.size());
	for (const auto& type : types) {
		auto target = std::lower_bound(infos.begin(), infos.end(), type, std::less<>{});
		if (target == infos.end() || target->ID != type) {
			cmpts.emplace_back(type, nullptr);
			sizes.push_back(0);
			continue;
		}

		cmpts.emplace_back(type, (std::uint8_t*)data + target->offset);
		auto idx = static_cast<std::size_t>(std::distance(infos.begin(), target));
		if (type.GetAccessMode() == AccessMode::WRITE)
			infos[idx].version = GetHead()->archetype->Version();
		sizes.push_back(GetHead()->archetype->GetCmptTraits().GetTraits()[idx].size);
	}

	return {
		(Entity*)GetCmptArray(TypeID_of<Entity>),
		cmpts,
		sizes
	};
}

void Chunk::Head::UpdateVersion(std::uint64_t version) {
	order_version = version;
	for (auto& info : GetCmptInfos())
		info.version = version;
}
