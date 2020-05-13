#pragma once

#include <cassert>

namespace Ubpa {
	template<typename... Cmpts>
	Archetype::Archetype(TypeList<Cmpts...>) noexcept
		: types(TypeList<Entity, Cmpts...>{})
	{
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"Archetype::Archetype: <Cmpts> must be different");
		cmptTraits.Register<Entity>();
		(cmptTraits.Register<Cmpts>(), ...);
		SetLayout();
	}

	template<typename... Cmpts>
	static Archetype* Archetype::Add(Archetype* from) noexcept {
		using CmptList = TypeList<Cmpts...>;
		assert((from->types.IsNotContain<Cmpts>() &&...));

		Archetype* rst = new Archetype;
		
		rst->types = from->types;
		rst->cmptTraits = from->cmptTraits;

		rst->types.Insert<Cmpts...>();
		(rst->cmptTraits.Register<Cmpts>(), ...);

		rst->SetLayout();
		
		return rst;
	}

	template<typename... Cmpts>
	static Archetype* Archetype::Remove(Archetype* from) noexcept {
		using CmptList = TypeList<Cmpts...>;
		assert((from->types.IsContain<Cmpts>() &&...));

		Archetype* rst = new Archetype;

		rst->types = from->types;
		rst->cmptTraits = from->cmptTraits;

		rst->types.Erase<Cmpts...>();
		(rst->cmptTraits.Deregister<Cmpts>(), ...);

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

		size_t idx = RequestBuffer();
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();

		new(buffer + Offsetof(CmptType::Of<Entity>()) + idxInChunk * sizeof(Entity))Entity(e);

		std::tuple<Cmpts*...> cmpts = { new(buffer + Offsetof(CmptType::Of<Cmpts>()) + idxInChunk * sizeof(Cmpts))Cmpts... };

		return { idx,cmpts };
	}

	template<typename... Cmpts>
	const std::vector<std::tuple<Entity*, Cmpts*...>> Archetype::Locate() const {
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"Archetype::Archetype: <Cmpts> must be different");
		assert((types.IsContain<Cmpts>() &&...));

		using CmptList = TypeList<Cmpts...>;

		auto offsets = std::make_tuple(Offsetof(CmptType::Of<Cmpts>())...);

		auto offset_Entity = Offsetof(CmptType::Of<Entity>());

		std::vector<std::tuple<Entity*, Cmpts*...>> rst;
		for (auto chunk : chunks) {
			auto ptr_Entity = reinterpret_cast<Entity*>(chunk->Data() + offset_Entity);
			rst.emplace_back(ptr_Entity, reinterpret_cast<Cmpts*>(chunk->Data() + std::get<Find_v<CmptList, Cmpts>>(offsets))...);
		}

		return rst;
	}
}
