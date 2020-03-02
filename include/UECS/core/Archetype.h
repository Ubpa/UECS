#pragma once

#include "Chunk.h"

#include <UTemplate/TypeID.h>

#include <vector>
#include <tuple>
#include <map>
#include <set>
#include <cassert>

namespace Ubpa {
	class ArchetypeMngr;

	class Archetype {
	public:
		struct ID : private std::set<size_t> {
			ID() = default;
			template<typename... Cmpts>
			ID(TypeList<Cmpts...>) noexcept { Add<Cmpts...>(); }

			template<typename... Cmpts>
			void Add() noexcept { (insert(TypeID<Cmpts>),...); }

			template<typename... Cmpts>
			bool IsContain() const noexcept {
				return ((find(TypeID<Cmpts>) != end()) &&...);
			}

			template<typename... Cmpts>
			bool Is() const noexcept {
				return sizeof...(Cmpts) == size() && IsContain<Cmpts...>();
			}

			using std::set<size_t>::begin;
			using std::set<size_t>::end;

			bool operator<(const ID& id) const noexcept;
			bool operator==(const ID& id) const noexcept;
		};

		// argument is for type deduction
		template<typename... Cmpts>
		Archetype(ArchetypeMngr* mngr, TypeList<Cmpts...>) noexcept;

		template<typename Cmpt>
		Archetype(Archetype* srcArchetype, IType<Cmpt>) noexcept;

		inline ~Archetype(){
			for (auto c : chunks)
				delete c;
		}

		template<typename... Cmpts>
		inline const std::tuple<std::vector<Cmpts*>...> Locate() {
			return { LocateOne<Cmpts>()... };
		}

		std::tuple<void*, size_t> At(size_t cmptHash, size_t idx);

		template<typename Cmpt>
		Cmpt* At(size_t idx);

		// no init
		inline size_t CreateEntity() {
			if (num % chunkCapacity == 0)
				chunks.push_back(new Chunk);
			return num++;
		}

		// init all cmpts
		template<typename... Cmpts>
		size_t CreateEntity();

		// erase idx-th entity
		// if idx != num-1, back entity will put at idx, return num-1
		// else return static_cast<size_t>(-1)
		size_t Erase(size_t idx);

		inline size_t Size() const noexcept { return num; }
		inline size_t ChunkNum() const noexcept { return chunks.size(); }
		inline size_t ChunkCapacity() const noexcept { return chunkCapacity; }
		inline const ID& GetID() const noexcept { return id; }
		inline ArchetypeMngr* GetArchetypeMngr() const noexcept { return mngr; }

		template<typename... Cmpts>
		inline bool IsContain() const noexcept {
			return id.IsContain<Cmpts...>();
		}

		template<typename Cmpt, typename... Args>
		inline Cmpt* Init(size_t idx, Args&&... args) noexcept {
			Cmpt* cmpt = reinterpret_cast<Cmpt*>(At<Cmpt>(idx));
			new (cmpt) Cmpt(std::forward<Args>(args)...);
			return cmpt;
		}

	private:
		template<typename Cmpt>
		const std::vector<Cmpt*> LocateOne();

	private:
		friend class Entity;

		ArchetypeMngr* mngr;
		ID id;
		std::map<size_t, std::tuple<size_t, size_t>> h2so; // hash to {size, offset}
		size_t chunkCapacity;
		std::vector<Chunk*> chunks;
		size_t num{ 0 };
	};
}

#include "detail/Archetype.inl"
