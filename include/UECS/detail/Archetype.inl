#pragma once

#include <cassert>

namespace Ubpa::UECS {
	template<typename... Cmpts>
	Archetype::Archetype(EntityMngr* entityMngr, TypeList<Cmpts...>)
		:
		entityMngr{ entityMngr },
		types(GenCmptTypeSet<Cmpts...>())
	{
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"<Cmpts>... must be different");
		cmptTraits.Register<Entity>();
		(cmptTraits.Register<Cmpts>(), ...);
		SetLayout();
	}

	template<typename... Cmpts>
	Archetype* Archetype::Add(const Archetype* from) {
		static_assert(sizeof...(Cmpts) > 0);
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"<Cmpts>... must be different");
		assert(!from->types.Contains(std::array{ CmptType::Of<Cmpts>... }));

		Archetype* rst = new Archetype{ from->entityMngr };
		
		rst->types = from->types;
		(rst->types.data.insert(CmptType::Of<Cmpts>), ...);
		rst->cmptTraits = from->cmptTraits;
		(rst->cmptTraits.Register<Cmpts>(), ...);

		rst->SetLayout();
		
		return rst;
	}

	template<typename... Cmpts>
	std::tuple<size_t, std::tuple<Cmpts *...>> Archetype::Create(Entity e) {
		static_assert((std::is_constructible_v<Cmpts> &&...),
			"<Cmpts> isn't constructible");
		static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
			"<Cmpts>... must be different");

		assert((types.Contains(CmptType::Of<Cmpts>) &&...) && types.data.size() == 1 + sizeof...(Cmpts));

		size_t idx = RequestBuffer();
		size_t idxInChunk = idx % chunkCapacity;
		byte* buffer = chunks[idx / chunkCapacity]->Data();

		new(buffer + Offsetof(CmptType::Of<Entity>) + idxInChunk * sizeof(Entity))Entity(e);

		std::tuple cmpts = { new(buffer + Offsetof(CmptType::Of<Cmpts>) + idxInChunk * sizeof(Cmpts))Cmpts... };

		return { idx,cmpts };
	}

	template<typename... Cmpts>
	CmptTypeSet Archetype::GenCmptTypeSet() {
		if constexpr (sizeof...(Cmpts) > 0) {
			static_assert(IsSet_v<TypeList<Entity, Cmpts...>>,
				"<Cmpts>... must be different");

			constexpr std::array types = { CmptType::Of<Cmpts>... };
			return Archetype::GenCmptTypeSet(types.data(), types.size());
		}
		else
			return Archetype::GenCmptTypeSet(nullptr, 0);
	}
}
