#pragma once

#include "CmptType.h"

#include <UContainer/Span.h>

namespace Ubpa::UECS {
	class Archetype;

	class ChunkView {
	public:
		ChunkView(Archetype* archetype, size_t chunkIdx) noexcept;
		ChunkView() noexcept = default;

		bool Contains(CmptType) const;
		size_t EntityNum() const noexcept { return entityNum; }

		// nullptr if not contain
		void* GetCmptArray(CmptType) const;
		template<typename Cmpt>
		Span<Cmpt> GetCmptArray() const;
		Span<const Entity> GetEntityArray() const;

	private:
		Archetype* archetype{ nullptr };
		size_t chunkIdx{ static_cast<size_t>(-1) };
		size_t entityNum{ 0 };
	};
}

#include "detail/ChunkView.inl"
