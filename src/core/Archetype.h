#pragma once

#include "ArchetypeCmptTraits.h"
#include "Chunk.h"

#include <UECS/Entity.h>
#include <UECS/CmptPtr.h>
#include <UECS/CmptLocator.h>
#include <UECS/details/TypeIDSet.h>

#include <UTemplate/TypeList.h>
#include <memory_resource>

namespace Ubpa::UECS {
	class EntityMngr;

	// Entity is a special Component
	// type of Entity + Components is Archetype's type
	class Archetype {
	public:
		Archetype(std::pmr::polymorphic_allocator<Chunk> chunkAllocator) noexcept : chunkAllocator{ chunkAllocator } {}

		// copy
		Archetype(std::pmr::polymorphic_allocator<Chunk> chunkAllocator, const Archetype&);
		Archetype(const Archetype&) = delete;

		~Archetype();

		// auto add Entity
		static Archetype* New(RTDCmptTraits&, std::pmr::polymorphic_allocator<Chunk> chunkAllocator, std::span<const TypeID> types);

		static Archetype* Add(RTDCmptTraits&, const Archetype* from, std::span<const TypeID> types);

		// auto add Entity
		static Archetype* Remove(const Archetype* from, std::span<const TypeID> types);

		// Entity + Components
		std::tuple<
			small_vector<Entity*, 16>,
			small_vector<small_vector<CmptAccessPtr, 16>, 16>,
			small_vector<std::size_t, 16>
		>
		Locate(const CmptLocator& locator) const;

		// nullptr if not contains
		void* Locate(std::size_t chunkIdx, TypeID) const;

		Chunk* GetChunk(std::size_t chunkIdx) const { return chunks[chunkIdx]; }
		
		// nullptr if not contains
		void* At(TypeID, std::size_t idx) const;

		// nullptr if not contains
		template<typename Cmpt>
		Cmpt* At(std::size_t idx) const{ return reinterpret_cast<Cmpt*>(At(TypeID_of<Cmpt>, idx)); }

		// no Entity
		std::vector<CmptPtr> Components(std::size_t idx) const;

		// no init
		std::size_t RequestBuffer();

		std::size_t Create(RTDCmptTraits&, Entity);
		
		// return index in archetype
		std::size_t Instantiate(Entity, std::size_t srcIdx);

		// erase idx-th entity
		// if idx != num-1, back entity will put at idx, return moved Entity's index
		// else return static_cast<std::size_t>(-1)
		// move-assignment + destructor
		std::size_t Erase(std::size_t idx);

		// Components + Entity
		const TypeIDSet& GetTypeIDSet() const noexcept { return types; }
		const ArchetypeCmptTraits& GetArchetypeCmptTraits() const noexcept { return cmptTraits; }

		std::size_t EntityNum() const noexcept { return entityNum; }
		std::size_t EntityNumOfChunk(std::size_t chunkIdx) const noexcept;
		std::size_t ChunkNum() const noexcept { return chunks.size(); }
		std::size_t ChunkCapacity() const noexcept { return chunkCapacity; }

		// add Entity
		static TypeIDSet GenTypeIDSet(std::span<const TypeID> types);

	private:
		// set type2alignment
		// call after setting type2size and type2offset
		void SetLayout();

		static bool NotContainEntity(std::span<const TypeID> types) noexcept;

		friend class EntityMngr;

		TypeIDSet types; // Entity + Components
		ArchetypeCmptTraits cmptTraits;

		std::size_t chunkCapacity{ static_cast<std::size_t>(-1) };
		std::pmr::polymorphic_allocator<Chunk> chunkAllocator;
		std::vector<Chunk*> chunks;

		std::size_t entityNum{ 0 }; // number of entities
	};
}
