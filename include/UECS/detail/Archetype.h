#pragma once

#include "../CmptPtr.h"

#include "Chunk.h"
#include "CmptTypeSet.h"

#include <UTemplate/Typelist.h>
#include <UTemplate/TypeID.h>

#include <map>

namespace Ubpa {
	class EntityMngr;
	class Archetype {
	public:
		// argument TypeList<Cmpts...> is for type deduction
		template<typename... Cmpts>
		Archetype(EntityMngr* mngr, TypeList<Cmpts...>) noexcept;

		template<typename... Cmpts>
		static Archetype* Add(Archetype* from) noexcept;
		template<typename... Cmpts>
		static Archetype* Remove(Archetype* from) noexcept;

		~Archetype();

		template<typename... Cmpts>
		const std::vector<std::tuple<Cmpts*...>> Locate() const;

		std::tuple<std::vector<std::vector<void*>>, std::vector<size_t>> Locate(const std::set<CmptType>& cmptTypes) const;
		
		std::tuple<void*, size_t> At(CmptType type, size_t idx) const;

		template<typename Cmpt>
		Cmpt* At(size_t idx) const;

		std::vector<CmptPtr> Components(size_t idx) const;

		// no init
		size_t RequestBuffer();

		// init cmpts
		template<typename... Cmpts>
		const std::tuple<size_t, std::tuple<Cmpts*...>> CreateEntity();

		// erase idx-th entity
		// if idx != num-1, back entity will put at idx, return num-1
		// else return static_cast<size_t>(-1)
		size_t Erase(size_t idx);

		EntityMngr* GetEntityMngr() const noexcept { return mngr; }

		const CmptTypeSet& GetCmptTypeSet() const noexcept { return cmptTypeSet; }
		size_t CmptNum() const noexcept { return cmptTypeSet.size(); }

		size_t EntityNum() const noexcept { return entityNum; }
		size_t ChunkNum() const noexcept { return chunks.size(); }
		size_t ChunkCapacity() const noexcept { return chunkCapacity; }

	private:
		Archetype() = default;

		friend class Entity;
		friend class EntityMngr;

		EntityMngr* mngr{ nullptr };

		CmptTypeSet cmptTypeSet;
		std::map<CmptType, std::tuple<size_t, size_t>> type2so; // CmptType to (size, offset)

		size_t chunkCapacity;
		std::vector<Chunk*> chunks;

		size_t entityNum{ 0 }; // number of entities
	};
}

#include "Archetype.inl"
