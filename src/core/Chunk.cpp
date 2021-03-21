#include <UECS/Chunk.hpp>

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

void Chunk::Head::UpdateVersion(std::uint64_t version) {
	order_version = version;
	for (auto& info : GetCmptInfos())
		info.version = version;
}
