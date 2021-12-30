#pragma once

#include "Entity.hpp"
#include "CmptPtr.hpp"
#include "config.hpp"

#include <span>
#include <memory_resource>

namespace Ubpa::UECS {
	class Archetype;

	class alignas(ChunkAlignment) Chunk {
	public:
		bool Contains(TypeID) const noexcept;

		bool DidChange(TypeID cmptType, std::size_t version) const noexcept;
		bool DidOrderChange(std::size_t version) const noexcept;

		std::uint64_t GetComponentVersion(TypeID cmptType) const noexcept;
		std::uint64_t GetOrderVersion() const noexcept;

		std::size_t EntityNum() const noexcept;
		std::size_t ComponentNum() const noexcept;

		// if not contains the component, return nullptr
		void* GetCmptArray(TypeID cmptType) const noexcept;

		// if not contains the component, return nullptr
		template<typename Cmpt>
		std::span<Cmpt> GetCmptArray() const noexcept {
			void* ptr = GetCmptArray(TypeID_of<Cmpt>);
			if (ptr) return { (Cmpt*)ptr,EntityNum() };
			else return {};
		}

		std::span<Entity> GetEntityArray() const noexcept;

		bool Full() const noexcept;
		bool Empty() const noexcept;

		bool HasAnyChange(std::span<const TypeID> types, std::uint64_t version) const noexcept;

		void ApplyChanges(std::span<const AccessTypeID> types);

		// ApplyChanges
		std::tuple<Entity*, small_vector<CmptAccessPtr>, small_vector<std::size_t>> Locate(std::span<const AccessTypeID> types);

		std::pmr::unsynchronized_pool_resource* GetChunkUnsyncResource() noexcept;
		std::pmr::monotonic_buffer_resource* GetChunkUnsyncFrameResource() noexcept;

		template<typename T, typename... Args>
		T* ChunkUnsyncNewFrameObject(Args&&... args) {
			auto rsrc = GetChunkUnsyncFrameResource();
			auto obj = (T*)rsrc->allocate(sizeof(T), alignof(T));
			std::pmr::polymorphic_allocator{ rsrc }.construct(obj, std::forward<Args>(args)...);
			return obj;
		}
		
	private:
		friend class Archetype;

		struct Head {
			std::pmr::unsynchronized_pool_resource chunk_unsync_rsrc;
			std::aligned_storage_t<sizeof(std::pmr::monotonic_buffer_resource)> chunk_unsync_frame_rsrc;
			Archetype* archetype;
			std::uint64_t num_entity;
			std::uint64_t num_component;
			std::uint64_t capacity;
			std::uint64_t order_version;

			struct CmptInfo {
				TypeID ID;
				std::uint64_t offset;
				std::uint64_t version;
				friend bool operator<(const CmptInfo& lhs, const CmptInfo& rhs) noexcept { return lhs.ID < rhs.ID; }
				friend bool operator<(const CmptInfo& lhs, const TypeID& rhs) noexcept { return lhs.ID < rhs; }
				friend bool operator<(const TypeID& lhs, const CmptInfo& rhs) noexcept { return lhs < rhs.ID; }
			}; // 24 bytes
			static_assert(sizeof(CmptInfo) == 24);

			// sorted by ID
			std::span<CmptInfo> GetCmptInfos() noexcept;
			std::span<const CmptInfo> GetCmptInfos() const noexcept;
		};

		Chunk() noexcept = default;
		~Chunk();

		Head* GetHead() noexcept;
		const Head* GetHead() const noexcept;

		std::size_t Erase(std::size_t idx);

		void ForceUpdateVersion(std::uint64_t version);

		static_assert(ChunkSize > sizeof(Head));
		std::uint8_t data[ChunkSize];
	};

	class ChunkView {
	public:
		ChunkView(const Chunk* c = nullptr) : chunk { c }{}
		const Chunk* GetChunk() const noexcept { return chunk; }
		const Chunk* operator->() const noexcept { return chunk; }
	private:
		const Chunk* chunk;
	};
}
