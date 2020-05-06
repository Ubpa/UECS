#pragma once

#include "RuntimeCmptTraits.h"

#include <cassert>

namespace Ubpa {
	template<typename... Cmpts>
	Archetype::Archetype(EntityMngr* mngr, TypeList<Cmpts...>) noexcept
		: mngr(mngr), id(TypeList<Cmpts...>{})
	{
		using CmptList = TypeList<Cmpts...>;

		constexpr auto info = Chunk::StaticInfo<Cmpts...>();
		chunkCapacity = info.capacity;
		((id2so[TypeID<Cmpts>] = std::make_pair(info.sizes[Find_v<CmptList, Cmpts>], info.offsets[Find_v<CmptList, Cmpts>])), ...);
	}

	template<typename... Cmpts>
	static Archetype* Archetype::Add(Archetype* from) noexcept {
		using CmptList = TypeList<Cmpts...>;
		assert(((!from->id.IsContain<Cmpts>()) &&...));

		Archetype* rst = new Archetype;
		rst->mngr = from->mngr;

		rst->id = from->id;
		rst->id.Add<Cmpts...>();

		std::vector<size_t> ids;
		std::vector<size_t> s;
		((ids.push_back(TypeID<Cmpts>), s.push_back(sizeof(Cmpts))), ...);
		for (auto [id, so] : from->id2so) {
			ids.push_back(id);
			s.push_back(std::get<0>(so));
		}
		auto co = Chunk::CO(s);
		rst->chunkCapacity = std::get<0>(co);
		((rst->id2so[TypeID<Cmpts>] = std::make_pair(s[Find_v<CmptList, Cmpts>], std::get<1>(co)[Find_v<CmptList, Cmpts>])), ...);
		for (size_t i = sizeof...(Cmpts); i < rst->id.size(); i++)
			rst->id2so[ids[i]] = std::make_pair(s[i], std::get<1>(co)[i]);
		return rst;
	}

	template<typename... Cmpts>
	static Archetype* Archetype::Remove(Archetype* from) noexcept {
		using CmptList = TypeList<Cmpts...>;
		assert((from->id.IsContain<Cmpts>() &&...));

		Archetype* rst = new Archetype;
		rst->mngr = from->mngr;

		rst->id = from->id;
		rst->id.Remove<Cmpts...>();

		std::vector<size_t> ids;
		std::vector<size_t> s;
		for (auto [id, so] : from->id2so) {
			if (!rst->id.IsContain(id))
				continue;
			ids.push_back(id);
			s.push_back(std::get<0>(so));
		}
		auto co = Chunk::CO(s);
		rst->chunkCapacity = std::get<0>(co);
		for (size_t i = 0; i < rst->id.size(); i++)
			rst->id2so[ids[i]] = std::make_pair(s[i], std::get<1>(co)[i]);
		return rst;
	}

	template<typename Cmpt>
	Cmpt* Archetype::At(size_t idx) const {
		auto [ptr, size] = At(TypeID<Cmpt>, idx);
		assert(ptr != nullptr);
		assert(size == sizeof(Cmpt));
		return reinterpret_cast<Cmpt*>(ptr);
	}

	template<typename... Cmpts>
	const std::tuple<size_t, std::tuple<Cmpts *...>> Archetype::CreateEntity() {
		assert((id.IsContain<Cmpts>() &&...) && id.size() == sizeof...(Cmpts));
		static_assert((std::is_constructible_v<Cmpts> &&...),
			"Archetype::CreateEntity: <Cmpts> isn't constructible");
		static_assert(IsSet_v<TypeList<Cmpts...>>,
			"Archetype::CreateEntity: <Cmpts> must be different");

		using CmptList = TypeList<Cmpts...>;
		size_t idx = RequestBuffer();
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();
		auto soTuple = std::make_tuple(id2so.find(TypeID<Cmpts>)->second...);
		std::tuple<Cmpts *...> cmpts = { new(buffer + std::get<1>(std::get<Find_v<CmptList,Cmpts>>(soTuple)) + idxInChunk * std::get<0>(std::get<Find_v<CmptList,Cmpts>>(soTuple)))Cmpts... };

		return { idx,cmpts };
	}

	template<typename... Cmpts>
	const std::vector<std::tuple<Cmpts*...>> Archetype::Locate() const {
		using CmptList = TypeList<Cmpts...>;
		auto targets = std::make_tuple(id2so.find(TypeID<Cmpts>)...);
		assert(((std::get<Find_v<CmptList, Cmpts>>(targets) != id2so.end())&&...));
		assert(((sizeof(Cmpts) == std::get<0>(std::get<Find_v<CmptList, Cmpts>>(targets)->second))&&...));
		auto offsets = std::make_tuple(std::get<1>(std::get<Find_v<CmptList, Cmpts>>(targets)->second)...);
		std::vector<std::tuple<Cmpts*...>> rst;
		for (auto chunk : chunks)
			rst.emplace_back(reinterpret_cast<Cmpts*>(chunk->Data() + std::get<Find_v<CmptList, Cmpts>>(offsets))...);
		return rst;
	}

	template<typename... Cmpts>
	inline bool Archetype::IsContain() const noexcept {
		return id.IsContain<Cmpts...>();
	}
}
