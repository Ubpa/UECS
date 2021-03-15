#pragma once

#include <cassert>

namespace Ubpa::UECS {
	template<typename... Cmpts>
	Archetype::Archetype(std::pmr::polymorphic_allocator<Chunk> chunkAllocator, TypeList<Cmpts...>) :
		types(GenTypeIDSet<Cmpts...>()),
		chunkAllocator{ chunkAllocator }
	{
		static_assert(IsUnique_v<TypeList<Entity, Cmpts...>>,
			"<Cmpts>... must be different");
		cmptTraits.Register<Entity>();
		(cmptTraits.Register<Cmpts>(), ...);
		SetLayout();
	}

	template<typename... Cmpts>
	Archetype* Archetype::Add(const Archetype* from) {
		static_assert(sizeof...(Cmpts) > 0);
		static_assert(IsUnique_v<TypeList<Entity, Cmpts...>>,
			"<Cmpts>... must be different");
		assert(!(from->types.Contains(TypeID_of<Cmpts>) &&...));

		Archetype* rst = new Archetype{ from->chunkAllocator };
		
		rst->types = from->types;
		(rst->types.data.insert(TypeID_of<Cmpts>), ...);
		rst->cmptTraits = from->cmptTraits;
		(rst->cmptTraits.Register<Cmpts>(), ...);

		rst->SetLayout();
		
		return rst;
	}

	template<typename... Cmpts>
	std::tuple<std::size_t, std::tuple<Cmpts *...>> Archetype::Create(Entity e) {
		static_assert((std::is_constructible_v<Cmpts> &&...),
			"<Cmpts> isn't constructible");
		static_assert(IsUnique_v<TypeList<Entity, Cmpts...>>,
			"<Cmpts>... must be different");

		assert((types.Contains(TypeID_of<Cmpts>) &&...) && types.data.size() == 1 + sizeof...(Cmpts));

		std::size_t idx = RequestBuffer();
		std::size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();

		new(buffer + Offsetof(TypeID_of<Entity>) + idxInChunk * sizeof(Entity))Entity(e);

		std::tuple cmpts = { new(buffer + Offsetof(TypeID_of<Cmpts>) + idxInChunk * sizeof(Cmpts))Cmpts... };

		return { idx,cmpts };
	}

	template<typename... Cmpts>
	TypeIDSet Archetype::GenTypeIDSet() {
		if constexpr (sizeof...(Cmpts) > 0) {
			static_assert(IsUnique_v<TypeList<Entity, Cmpts...>>,
				"<Cmpts>... must be different");

			constexpr std::array types = { TypeID_of<Cmpts>... };
			return GenTypeIDSet(types);
		}
		else
			return GenTypeIDSet({});
	}
}
