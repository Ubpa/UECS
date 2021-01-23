#pragma once

#include "AccessTypeID.h"
#include "Entity.h"

#include <span>

namespace Ubpa::UECS {
	class Archetype;

	class ChunkView {
	public:
		ChunkView(Archetype* archetype, std::size_t chunkIdx) noexcept;
		ChunkView() noexcept = default;

		bool Contains(TypeID) const;
		std::size_t EntityNum() const noexcept { return entityNum; }

		// nullptr if not contain
		void* GetCmptArray(TypeID) const;
		template<typename Cmpt>
		std::span<Cmpt> GetCmptArray() const;
		std::span<const Entity> GetEntityArray() const;

	private:
		Archetype* archetype{ nullptr };
		std::size_t chunkIdx{ static_cast<std::size_t>(-1) };
		std::size_t entityNum{ 0 };
	};
}

#include "detail/ChunkView.inl"
