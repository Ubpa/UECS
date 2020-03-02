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

	template<typename... Cmpts>
	Archetype* Archetype::Add<Cmpts...>::From(Archetype* srcArchetype) noexcept {
		using CmptList = TypeList<Cmpts...>;
		assert(((!srcArchetype->id.IsContain<Cmpts>())&&...));

		Archetype* rst = new Archetype;
		rst->mngr = srcArchetype->mngr;

		rst->id = srcArchetype->id;
		rst->id.Add<Cmpts...>();

		std::vector<size_t> h;
		std::vector<size_t> s;
		((h.push_back(TypeID<Cmpts>), s.push_back(sizeof(Cmpts))), ...);
		for (auto p : srcArchetype->h2so) {
			h.push_back(p.first);
			s.push_back(std::get<0>(p.second));
		}
		auto co = Chunk::CO(s);
		rst->chunkCapacity = std::get<0>(co);
		((rst->h2so[TypeID<Cmpts>]=std::make_tuple(s[Find_v<CmptList,Cmpts>], std::get<1>(co)[Find_v<CmptList, Cmpts>])), ...);
		for (size_t i = sizeof...(Cmpts); i < rst->id.size(); i++)
			rst->h2so[h[i]] = std::make_tuple(s[i], std::get<1>(co)[i]);
		return rst;
	}

	template<typename... Cmpts>
	Archetype* Archetype::Remove<Cmpts...>::From(Archetype* srcArchetype) noexcept {
		using CmptList = TypeList<Cmpts...>;
		assert((srcArchetype->id.IsContain<Cmpts>() &&...));

		Archetype* rst = new Archetype;
		rst->mngr = srcArchetype->mngr;

		rst->id = srcArchetype->id;
		rst->id.Remove<Cmpts...>();

		std::vector<size_t> h;
		std::vector<size_t> s;
		for (auto p : rst->h2so) {
			if (rst->id.IsContain(p.first))
				continue;
			h.push_back(p.first);
			s.push_back(std::get<0>(p.second));
		}
		auto co = Chunk::CO(s);
		rst->chunkCapacity = std::get<0>(co);
		for (size_t i = 0; i < rst->id.size(); i++)
			rst->h2so[h[i]] = std::make_tuple(s[i], std::get<1>(co)[i]);
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
