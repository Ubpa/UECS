#pragma once

namespace Ubpa {
	template<typename... Cmpts>
	Archetype::Archetype(ArchetypeMngr* mngr, TypeList<Cmpts...>) noexcept : mngr(mngr), id(TypeList<Cmpts...>{}) {
		using CmptList = TypeList<Cmpts...>;

		constexpr size_t N = sizeof...(Cmpts);

		constexpr auto info = Chunk::StaticInfo<Cmpts...>();
		chunkCapacity = info.capacity;
		((h2so[TypeID<Cmpts>] = std::make_pair(info.sizes[Find_v<CmptList, Cmpts>], info.offsets[Find_v<CmptList, Cmpts>])), ...);
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
		((rst->h2so[TypeID<Cmpts>]=std::make_pair(s[Find_v<CmptList,Cmpts>], std::get<1>(co)[Find_v<CmptList, Cmpts>])), ...);
		for (size_t i = sizeof...(Cmpts); i < rst->id.size(); i++)
			rst->h2so[h[i]] = std::make_pair(s[i], std::get<1>(co)[i]);
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
			rst->h2so[h[i]] = std::make_pair(s[i], std::get<1>(co)[i]);
		return rst;
	}

	template<typename Cmpt>
	Cmpt* Archetype::At(size_t idx) {
		auto target = h2so.find(TypeID<Cmpt>);
		if (target == h2so.end())
			return nullptr;
		assert(sizeof(Cmpt) == target->second.first);
		size_t offset = target->second.second;
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();
		return reinterpret_cast<Cmpt*>(buffer + offset + sizeof(Cmpt) * idxInChunk);
	}

	template<typename... Cmpts>
	const std::pair<size_t, std::tuple<Cmpts *...>> Archetype::CreateEntity(EntityData* e) {
		assert(id.Is<Cmpts...>());

		using CmptList = TypeList<Cmpts...>;
		size_t idx = CreateEntity();
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();
		std::array<std::pair<size_t, size_t>, sizeof...(Cmpts)> soArr{ h2so[TypeID<Cmpts>]... };
		std::tuple<Cmpts *...> cmpts = { New<Cmpts>(buffer + soArr[Find_v<CmptList, Cmpts>].second + idxInChunk * soArr[Find_v<CmptList, Cmpts>].first, e)... };

		return { idx,cmpts };
	}

	template<typename Cmpt>
	const std::vector<Cmpt*> Archetype::LocateOne() {
		auto target = h2so.find(TypeID<Cmpt>);
		assert(target != h2so.end());
		assert(sizeof(Cmpt) == target->second.first);
		const size_t offset = target->second.second;
		std::vector<Cmpt*> rst;
		for (auto c : chunks)
			rst.push_back(reinterpret_cast<Cmpt*>(c->Data() + offset));
		return rst;
	}

	template<typename Cmpt>
	Cmpt* Archetype::New(void* addr, EntityData* e) {
		Cmpt* cmpt;
		if constexpr (std::is_constructible_v<Cmpt, Entity*>)
			cmpt = new(addr)Cmpt(reinterpret_cast<Entity*>(e));
		else
			cmpt = new(addr)Cmpt;
		e->RegistCmptRelease(cmpt);
		return cmpt;
	}

	template<typename Cmpt>
	Cmpt* Archetype::New(size_t idx, EntityData* e) {
		return New<Cmpt>(At<Cmpt>(idx), e);
	}
}
