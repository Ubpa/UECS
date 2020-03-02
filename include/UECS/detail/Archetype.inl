#pragma once

namespace Ubpa {
	template<typename... Cmpts>
	Archetype::Archetype(ArchetypeMngr* mngr, TypeList<Cmpts...>) noexcept : mngr(mngr), id(TypeList<Cmpts...>{}) {
		using CmptList = TypeList<Cmpts...>;

		constexpr size_t N = sizeof...(Cmpts);

		constexpr auto info = Chunk::StaticInfo<Cmpts...>();
		chunkCapacity = info.capacity;
		((h2so[TypeID<Cmpts>] = std::make_tuple(info.sizes[Find_v<CmptList, Cmpts>], info.offsets[Find_v<CmptList, Cmpts>])), ...);
	}

	template<typename Cmpt>
	Archetype* Archetype::Add<Cmpt>::From(Archetype* srcArchetype) noexcept {
		Archetype* rst = new Archetype;
		rst->mngr = srcArchetype->mngr;

		rst->id = srcArchetype->id;
		rst->id.Add<Cmpt>();

		std::map<size_t, size_t> s2h; // size to hash
		for (auto h : rst->id) {
			if (h == TypeID<Cmpt>)
				s2h[sizeof(Cmpt)] = h;
			else
				s2h[std::get<0>(srcArchetype->h2so[h])] = h;
		}
		std::vector<size_t> sizes;
		for (auto p : s2h)
			sizes.push_back(p.first); // sorted
		auto co = Chunk::CO(sizes);
		rst->chunkCapacity = std::get<0>(co);
		size_t i = 0;
		for (auto h : rst->id) {
			rst->h2so[h] = std::make_tuple(sizes[i], std::get<1>(co)[i]);
			i++;
		}

		return rst;
	}

	template<typename Cmpt>
	Archetype* Archetype::Remove<Cmpt>::From(Archetype* srcArchetype) noexcept {
		assert(srcArchetype->id.IsContain<Cmpt>());

		Archetype* rst = new Archetype;
		rst->mngr = srcArchetype->mngr;

		rst->id = srcArchetype->id;
		rst->id.Remove<Cmpt>();

		std::map<size_t, size_t> s2h; // size to hash
		for (auto h : rst->id)
			s2h[std::get<0>(srcArchetype->h2so[h])] = h;
		std::vector<size_t> sizes;
		for (auto p : s2h)
			sizes.push_back(p.first); // sorted
		auto co = Chunk::CO(sizes);
		rst->chunkCapacity = std::get<0>(co);
		size_t i = 0;
		for (auto h : rst->id) {
			rst->h2so[h] = std::make_tuple(sizes[i], std::get<1>(co)[i]);
			i++;
		}

		return rst;
	}

	template<typename Cmpt>
	Cmpt* Archetype::At(size_t idx) {
		auto target = h2so.find(TypeID<Cmpt>);
		if (target == h2so.end())
			return nullptr;
		assert(sizeof(Cmpt) == std::get<0>(target->second));
		size_t offset = std::get<1>(target->second);
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();
		return reinterpret_cast<Cmpt*>(buffer + offset + sizeof(Cmpt) * idxInChunk);
	}

	template<typename... Cmpts>
	size_t Archetype::CreateEntity() {
		assert(id.Is<Cmpts...>());

		using CmptList = TypeList<Cmpts...>;
		size_t idx = CreateEntity();
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();
		std::array<std::tuple<size_t, size_t>, sizeof...(Cmpts)> soArr{ h2so[TypeID<Cmpts>]... };
		(new(buffer + std::get<1>(soArr[Find_v<CmptList, Cmpts>]) + idxInChunk * std::get<0>(soArr[Find_v<CmptList, Cmpts>])) Cmpts(), ...);

		return idx;
	}

	template<typename Cmpt>
	const std::vector<Cmpt*> Archetype::LocateOne() {
		auto target = h2so.find(TypeID<Cmpt>);
		assert(target != h2so.end());
		const size_t offset = std::get<1>(target->second);
		std::vector<Cmpt*> rst;
		for (auto c : chunks)
			rst.push_back(reinterpret_cast<Cmpt*>(c->Data() + offset));
		return rst;
	}
}
