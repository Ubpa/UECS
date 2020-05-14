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
	Archetype* Archetype::Add(const Archetype* from) noexcept {
		assert((from->types.IsNotContain<Cmpts>() &&...));

		Archetype* rst = new Archetype;
		
		rst->types = from->types;
		rst->types.Insert<Cmpts...>();
		rst->cmptTraits = from->cmptTraits;
		(rst->cmptTraits.Register<Cmpts>(), ...);

		rst->SetLayout();
		
		return rst;
	}

	template<typename... CmptTypes>
	Archetype* Archetype::Add(const Archetype* from, CmptTypes... types) noexcept {
		static_assert((std::is_same_v<CmptTypes, CmptType> &&...));
		assert((from->types.IsNotContain(types) &&...));

		Archetype* rst = new Archetype;

		rst->types = from->types;
		rst->types.Insert(types...);
		rst->cmptTraits = from->cmptTraits;
		(rst->cmptTraits.Register(types), ...);

		rst->SetLayout();

		return rst;
	}

	template<typename... Cmpts>
	Archetype* Archetype::Remove(const Archetype* from) noexcept {
		assert((from->types.IsContain<Cmpts>() &&...));

		Archetype* rst = new Archetype;

		rst->types = from->types;
		rst->types.Erase<Cmpts...>();
		rst->cmptTraits = from->cmptTraits;
		(rst->cmptTraits.Deregister<Cmpts>(), ...);

		rst->SetLayout();

		return rst;
	}

	template<typename... CmptTypes>
	Archetype* Archetype::Remove(const Archetype* from, CmptTypes... types) noexcept {
		static_assert((std::is_same_v<CmptTypes, CmptType> &&...));
		assert((from->types.IsContain(types) &&...));

		Archetype* rst = new Archetype;

		rst->types = from->types;
		rst->types.Erase(types...);
		rst->cmptTraits = from->cmptTraits;
		(rst->cmptTraits.Deregister(types), ...);

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
	std::tuple<size_t, std::tuple<Cmpts *...>> Archetype::CreateEntity(Entity e) {
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
}
