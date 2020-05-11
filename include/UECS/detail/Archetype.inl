#pragma once

#include "RuntimeCmptTraits.h"

#include <cassert>

namespace Ubpa {
	template<typename... Cmpts>
	Archetype::Archetype(TypeList<Cmpts...>) noexcept
		: types(TypeList<Entity, Cmpts...>{}),
		type2size{ {CmptType::Of<Entity>(), sizeof(Entity)}, {CmptType::Of<Cmpts>(), sizeof(Cmpts)}... },
		type2alignment{ {CmptType::Of<Entity>(), sizeof(Entity)}, {CmptType::Of<Cmpts>(), alignof(Cmpts)}... }
	{
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"Archetype::Archetype: <Cmpts> must be different");
		SetLayout();
	}

	template<typename... Cmpts>
	static Archetype* Archetype::Add(Archetype* from) noexcept {
		using CmptList = TypeList<Cmpts...>;
		assert((from->types.IsNotContain<Cmpts>() &&...));

		Archetype* rst = new Archetype;

		rst->types = from->types;
		rst->type2alignment = from->type2alignment;
		rst->type2size = from->type2size;

		rst->types.Add<Cmpts...>();
		((rst->type2alignment[CmptType::Of<Cmpts>()] = alignof(Cmpts)), ...);
		((rst->type2size[CmptType::Of<Cmpts>()] = sizeof(Cmpts)), ...);

		rst->SetLayout();
		
		return rst;
	}

	template<typename... Cmpts>
	static Archetype* Archetype::Remove(Archetype* from) noexcept {
		using CmptList = TypeList<Cmpts...>;
		assert((from->types.IsContain<Cmpts>() &&...));

		Archetype* rst = new Archetype;

		rst->types = from->types;
		rst->type2alignment = from->type2alignment;
		rst->type2size = from->type2size;

		rst->types.Remove<Cmpts...>();
		(rst->type2alignment.erase(CmptType::Of<Cmpts>()), ...);
		(rst->type2size.erase(CmptType::Of<Cmpts>()), ...);

		rst->SetLayout();

		return rst;
	}

	template<typename Cmpt>
	Cmpt* Archetype::At(size_t idx) const {
		auto [ptr, size] = At(CmptType::Of<Cmpt>(), idx);
		if (ptr == nullptr)
			return nullptr;
		assert(size == sizeof(Cmpt));
		return reinterpret_cast<Cmpt*>(ptr);
	}

	template<typename... Cmpts>
	const std::tuple<size_t, std::tuple<Cmpts *...>> Archetype::CreateEntity(Entity e) {
		assert((types.IsContain<Cmpts>() &&...) && types.size() == 1 + sizeof...(Cmpts));
		static_assert((std::is_constructible_v<Cmpts> &&...),
			"Archetype::CreateEntity: <Cmpts> isn't constructible");
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"Archetype::CreateEntity: <Cmpts> must be different");

		using CmptList = TypeList<Cmpts...>;
		size_t idx = RequestBuffer();
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();

		constexpr auto entityType = CmptType::Of<Entity>();
		new(buffer + type2offset[entityType] + idxInChunk * type2size[entityType])Entity(e);

		std::array<size_t, sizeof...(Cmpts)> sizes = { type2size.find(CmptType::Of<Cmpts>())->second... };
		std::array<size_t, sizeof...(Cmpts)> offsets = { type2offset.find(CmptType::Of<Cmpts>())->second... };
		std::tuple<Cmpts*...> cmpts = { new(buffer + offsets[Find_v<CmptList,Cmpts>] + idxInChunk * sizes[Find_v<CmptList,Cmpts>])Cmpts... };

		return { idx,cmpts };
	}

	template<typename... Cmpts>
	const std::vector<std::tuple<Entity*, Cmpts*...>> Archetype::Locate() const {
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"Archetype::Archetype: <Cmpts> must be different");
		assert((types.IsContain<Cmpts>() &&...));

		using CmptList = TypeList<Cmpts...>;

		auto offsets = std::make_tuple(type2offset.find(CmptType::Of<Cmpts>())->second...);

		auto offset_Entity = type2offset.find(CmptType::Of<Entity>())->second;

		std::vector<std::tuple<Entity*, Cmpts*...>> rst;
		for (auto chunk : chunks) {
			auto ptr_Entity = reinterpret_cast<Entity*>(chunk->Data() + offset_Entity);
			rst.emplace_back(ptr_Entity, reinterpret_cast<Cmpts*>(chunk->Data() + std::get<Find_v<CmptList, Cmpts>>(offsets))...);
		}

		return rst;
	}
}
