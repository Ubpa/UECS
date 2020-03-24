#pragma once

#include "Chunk.h"
#include "Pool.h"
#include "EntityBase.h"
#include "CmptMngr.h"

#include <UTemplate/TypeID.h>

#include <map>
#include <set>

namespace Ubpa {
	class ArchetypeMngr;
	class Entity;

	class Archetype {
	public:
		struct ID : private std::set<size_t> {
			ID() = default;
			template<typename... Cmpts>
			ID(TypeList<Cmpts...>) noexcept { Add<Cmpts...>(); }

			template<typename... Cmpts>
			void Add() noexcept { (insert(TypeID<Cmpts>),...); }
			template<typename... Cmpts>
			void Remove() noexcept { (erase(TypeID<Cmpts>), ...); }

			template<typename... Cmpts>
			bool IsContain() const noexcept {
				return ((find(TypeID<Cmpts>) != end()) &&...);
			}

			bool IsContain(size_t cmptHash) const noexcept {
				return find(cmptHash) != end();
			}

			template<typename... Cmpts>
			bool Is() const noexcept {
				return sizeof...(Cmpts) == size() && IsContain<Cmpts...>();
			}

			using std::set<size_t>::begin;
			using std::set<size_t>::end;

			bool operator<(const ID& id) const noexcept;
			bool operator==(const ID& id) const noexcept;

			friend class Archetype;
		};

		Archetype() = default;
		// argument is for type deduction
		template<typename... Cmpts>
		Archetype(ArchetypeMngr* mngr, TypeList<Cmpts...>) noexcept;

		template<typename... Cmpts>
		struct Add {
			static Archetype* From(Archetype* srcArchetype) noexcept;
		};
		template<typename... Cmpts>
		struct Remove {
			static Archetype* From(Archetype* srcArchetype) noexcept;
		};
		template<typename... Cmpts>
		friend struct Add;
		template<typename... Cmpts>
		friend struct Remove;

		~Archetype();

		template<typename... Cmpts>
		inline const std::tuple<std::vector<Cmpts*>...> Locate() {
			return { LocateOne<Cmpts>()... };
		}

		std::tuple<void*, size_t> At(size_t cmptHash, size_t idx);

		template<typename Cmpt>
		Cmpt* At(size_t idx);

		std::vector<std::tuple<void*, size_t>> Components(size_t idx);

		// no init
		inline size_t RequestBuffer() {
			if (num == chunks.size() * chunkCapacity)
				chunks.push_back(chunkPool.request());
			return num++;
		}

		// init cmpts
		template<typename... Cmpts>
		const std::tuple<size_t, std::tuple<Cmpts*...>> CreateEntity();

		// erase idx-th empty entity
		// if idx != num-1, back entity will put at idx, return num-1
		// else return static_cast<size_t>(-1)
		size_t Erase(size_t idx);

		inline size_t Size() const noexcept { return num; }
		inline size_t ChunkNum() const noexcept { return chunks.size(); }
		inline size_t ChunkCapacity() const noexcept { return chunkCapacity; }
		inline const ID& GetID() const noexcept { return id; }
		inline ArchetypeMngr* GetArchetypeMngr() const noexcept { return mngr; }
		inline size_t CmptNum() const noexcept { return id.size(); }

		template<typename... Cmpts>
		inline bool IsContain() const noexcept;

	private:
		template<typename Cmpt>
		const std::vector<Cmpt*> LocateOne();

	private:
		friend class Entity;
		friend class ArchetypeMngr;

		ArchetypeMngr* mngr;
		ID id;
		std::map<size_t, std::tuple<size_t, size_t>> h2so; // hash to (size, offset)
		size_t chunkCapacity;
		std::vector<Chunk*> chunks;
		size_t num{ 0 };

		static Pool<Chunk> chunkPool; // TODO: lock
	};
}

#include "Archetype.inl"
