#pragma once

#include "ArchetypeCmptTraits.hpp"

#include <UECS/Entity.hpp>
#include <UECS/CmptPtr.hpp>
#include <UECS/CmptLocator.hpp>

#include <memory_resource>

namespace Ubpa::UECS {
	class EntityMngr;

	// Entity is a special Component
	// type of Entity + Components is Archetype's type
	class Archetype {
	public:
		Archetype(std::pmr::memory_resource* rsrc) noexcept : chunkAllocator{ rsrc } {}

		// copy
		Archetype(std::pmr::memory_resource* rsrc, const Archetype&);
		Archetype(const Archetype&) = delete;

		~Archetype();

		// auto add Entity
		static Archetype* New(RTDCmptTraits&, std::pmr::memory_resource* rsrc, std::span<const TypeID> types);

		static Archetype* Add(RTDCmptTraits&, const Archetype* from, std::span<const TypeID> types);

		// auto add Entity
		static Archetype* Remove(const Archetype* from, std::span<const TypeID> types);

		// Entity + Components
		std::tuple<
			small_vector<Entity*, 16>,
			small_vector<small_vector<CmptAccessPtr, 16>, 16>,
			small_vector<std::size_t, 16>
		>
		Locate(std::span<const AccessTypeID> cmpts) const;

		// nullptr if not contains
		void* Locate(std::size_t chunkIdx, TypeID) const;
		
		// nullptr if not contains
		void* At(TypeID, std::size_t idx) const;

		// nullptr if not contains
		template<typename Cmpt>
		Cmpt* At(std::size_t idx) const{ return reinterpret_cast<Cmpt*>(At(TypeID_of<Cmpt>, idx)); }

		// no Entity
		std::vector<CmptPtr> Components(std::size_t idx) const;

		// no init
		std::size_t RequestBuffer();

		std::size_t Create(Entity);
		
		// return index in archetype
		std::size_t Instantiate(Entity, std::size_t srcIdx);

		// erase idx-th entity
		// if idx != num-1, back entity will put at idx, return moved Entity's index
		// else return static_cast<std::size_t>(-1)
		// move-assignment + destructor
		std::size_t Erase(std::size_t idx);

		// Components + Entity
		const ArchetypeCmptTraits& GetCmptTraits() const noexcept { return cmptTraits; }

		std::size_t EntityNum() const noexcept { return entityNum; }
		std::size_t EntityNumOfChunk(std::size_t chunkIdx) const noexcept;
		std::size_t ChunkNum() const noexcept { return chunks.size(); }
		std::size_t ChunkCapacity() const noexcept { return chunkCapacity; }

		// add Entity
		static small_flat_set<TypeID> GenTypeIDSet(std::span<const TypeID> types);

	private:
		// set type2alignment
		// call after setting type2size and type2offset
		void SetLayout();

		friend class EntityMngr;

		ArchetypeCmptTraits cmptTraits; // Entity + Components

		std::size_t chunkCapacity{ static_cast<std::size_t>(-1) };

		// std::aligned_storage_t<ChunkSize, ChunkAlignment>
		struct alignas(ChunkAlignment) Chunk { std::byte data[ChunkSize]; };
		std::pmr::polymorphic_allocator<Chunk> chunkAllocator;
		small_vector<Chunk*> chunks;
		small_vector<std::size_t> offsets;

		std::size_t entityNum{ 0 }; // number of entities
	};
}
