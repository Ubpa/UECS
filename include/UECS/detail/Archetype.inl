#pragma once

#include <cassert>

namespace Ubpa::UECS {
	template<typename... Cmpts>
	Archetype::Archetype(TypeList<Cmpts...>)
		: types(GenCmptTypeSet<Cmpts...>())
	{
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"Archetype::Archetype: <Cmpts>... must be different");
		cmptTraits.Register<Entity>();
		(cmptTraits.Register<Cmpts>(), ...);
		SetLayout();
	}

	template<typename... Cmpts>
	Archetype* Archetype::Add(const Archetype* from) {
		static_assert(sizeof...(Cmpts) > 0);
		assert(((!from->types.Contains(CmptType::Of<Cmpts>)) &&...));

		Archetype* rst = new Archetype;
		
		rst->types = from->types;
		rst->types.data.insert(CmptType::Of<Cmpts>...);
		rst->cmptTraits = from->cmptTraits;
		(rst->cmptTraits.Register<Cmpts>(), ...);

		rst->SetLayout();
		
		return rst;
	}

	template<typename... Cmpts>
	std::tuple<size_t, std::tuple<Cmpts *...>> Archetype::Create(Entity e) {
		assert((types.Contains(CmptType::Of<Cmpts>) &&...) && types.data.size() == 1 + sizeof...(Cmpts));
		static_assert((std::is_constructible_v<Cmpts> &&...),
			"Archetype::Create: <Cmpts> isn't constructible");
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"Archetype::Create: <Cmpts>... must be different");

		size_t idx = RequestBuffer();
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();

		new(buffer + Offsetof(CmptType::Of<Entity>) + idxInChunk * sizeof(Entity))Entity(e);

		std::tuple<Cmpts*...> cmpts = { new(buffer + Offsetof(CmptType::Of<Cmpts>) + idxInChunk * sizeof(Cmpts))Cmpts... };

		return { idx,cmpts };
	}

	template<typename... Cmpts>
	CmptTypeSet Archetype::GenCmptTypeSet() {
		if constexpr(sizeof...(Cmpts) == 0)
			return Archetype::GenCmptTypeSet(nullptr, 0);
		else {
			constexpr std::array types = { CmptType::Of<Cmpts>... };
			return Archetype::GenCmptTypeSet(types.data(), types.size());
		}
	}
}
