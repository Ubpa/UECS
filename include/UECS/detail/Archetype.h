#pragma once

#include "Chunk.h"
#include "EntityBase.h"
#include "CmptLifecycleMngr.h"
#include "CmptIDSet.h"

#include <UBL/Pool.h>

#include <UTemplate/Typelist.h>
#include <UTemplate/TypeID.h>

#include <map>
#include <set>

namespace Ubpa {
	class ArchetypeMngr;
	class Entity;

	class Archetype {
	public:
		// argument TypeList<Cmpts...> is for type deduction
		template<typename... Cmpts>
		Archetype(ArchetypeMngr* mngr, TypeList<Cmpts...>) noexcept;

		template<typename... Cmpts>
		static Archetype* Add(Archetype* from) noexcept;
		template<typename... Cmpts>
		static Archetype* Remove(Archetype* from) noexcept;

		~Archetype();

		template<typename... Cmpts>
		const std::vector<std::tuple<Cmpts*...>> Locate();

		std::tuple<void*, size_t> At(size_t cmptHash, size_t idx);

		template<typename Cmpt>
		Cmpt* At(size_t idx);

		std::vector<std::tuple<void*, size_t>> Components(size_t idx);

		// no init
		size_t RequestBuffer() {
			if (num == chunks.size() * chunkCapacity)
				chunks.push_back(chunkPool.Request());
			return num++;
		}

		// init cmpts
		template<typename... Cmpts>
		const std::tuple<size_t, std::tuple<Cmpts*...>> CreateEntity();

		// erase idx-th entity
		// if idx != num-1, back entity will put at idx, return num-1
		// else return static_cast<size_t>(-1)
		size_t Erase(size_t idx);

		inline size_t Size() const noexcept { return num; }
		inline size_t ChunkNum() const noexcept { return chunks.size(); }
		inline size_t ChunkCapacity() const noexcept { return chunkCapacity; }
		inline const CmptIDSet& ID() const noexcept { return id; }
		inline ArchetypeMngr* GetArchetypeMngr() const noexcept { return mngr; }
		inline size_t CmptNum() const noexcept { return id.size(); }

		template<typename... Cmpts>
		inline bool IsContain() const noexcept;

	private:
		Archetype() = default;

		friend class Entity;
		friend class ArchetypeMngr;

		ArchetypeMngr* mngr;
		CmptIDSet id;
		std::map<size_t, std::tuple<size_t, size_t>> h2so; // hash to (size, offset)
		size_t chunkCapacity;
		std::vector<Chunk*> chunks;
		size_t num{ 0 };

		Pool<Chunk> chunkPool;
	};
}

#include "Archetype.inl"
