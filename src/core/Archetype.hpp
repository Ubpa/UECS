#pragma once

#include "ArchetypeCmptTraits.hpp"

#include <UECS/Entity.hpp>
#include <UECS/CmptPtr.hpp>
#include <UECS/CmptLocator.hpp>
#include <UECS/Chunk.hpp>

#include <memory_resource>

namespace Ubpa::UECS {
	class EntityMngr;
	class World;

	// Entity is a special Component
	// type of Entity + Components is Archetype's type
	class Archetype {
	public:
		struct EntityAddress {
			std::size_t chunkIdx;
			std::size_t idxInChunk;
		};

		Archetype(World*);

		// copy
		Archetype(const Archetype&, World*);

		Archetype(const Archetype&) = delete;

		~Archetype();

		// auto add Entity
		static Archetype* New(CmptTraits&, World* world, std::span<const TypeID> types);

		static Archetype* Add(CmptTraits&, const Archetype* from, std::span<const TypeID> types);

		// auto add Entity
		static Archetype* Remove(const Archetype* from, std::span<const TypeID> types);

		// Entity + Components
		std::tuple<
			small_vector<Entity*>,
			small_vector<small_vector<CmptAccessPtr>>,
			small_vector<std::size_t>
		>
		Locate(std::span<const AccessTypeID> cmpts) const;
		
		// nullptr if not contains
		CmptAccessPtr At(AccessTypeID type, EntityAddress) const;
		CmptAccessPtr WriteAt(TypeID type, EntityAddress addr) const { return At({ type, AccessMode::WRITE }, addr); }
		CmptAccessPtr ReadAt(TypeID type, EntityAddress addr) const { return At({ type, AccessMode::LATEST }, addr); }

		// nullptr if not contains
		template<typename Cmpt>
		Cmpt* WriteAt(EntityAddress addr) const{ return WriteAt(TypeID_of<Cmpt>, addr).template As<Cmpt, AccessMode::WRITE>(); }
		template<typename Cmpt>
		const Cmpt* ReadAt(EntityAddress addr) const { return ReadAt(TypeID_of<Cmpt>, addr).template As<Cmpt, AccessMode::LATEST>(); }

		// no Entity
		std::vector<CmptAccessPtr> Components(EntityAddress addr, AccessMode mode) const;
		std::vector<CmptAccessPtr> WriteComponents(EntityAddress addr) const { return Components(addr, AccessMode::WRITE); }
		std::vector<CmptAccessPtr> ReadComponents(EntityAddress addr) const { return Components(addr, AccessMode::LATEST); }

		// no init
		EntityAddress RequestBuffer();

		EntityAddress Create(Entity);

		// return index in archetype
		EntityAddress Instantiate(Entity, EntityAddress src);
		
		// return moved entity index (moved to addr)
		std::size_t Erase(EntityAddress addr);

		// Components + Entity
		const ArchetypeCmptTraits& GetCmptTraits() const noexcept { return cmptTraits; }

		// add Entity
		static small_flat_set<TypeID> GenTypeIDSet(std::span<const TypeID> types);

		std::size_t EntityNum() const noexcept { return entityNum; }

		std::span<Chunk*> GetChunks() noexcept { return { chunks.data(), chunks.size() }; }
		std::span<std::size_t> GetOffsets() noexcept { return { offsets.data(), offsets.size() }; }

		void NewFrame();

		std::uint64_t Version() const noexcept;

	private:
		// set type2alignment
		// call after setting type2size and type2offset
		void SetLayout();

		friend class EntityMngr;

		World* world;

		ArchetypeCmptTraits cmptTraits; // Entity + Components

		// chunk infomations
		small_vector<Chunk*> chunks;
		std::size_t chunkCapacity{ static_cast<std::size_t>(-1) };
		small_vector<std::size_t> offsets; // component
		small_flat_set<std::size_t, 16, std::greater<std::size_t>> nonFullChunks; // big -> small, use small first

		std::size_t entityNum{ 0 };
	};
}
