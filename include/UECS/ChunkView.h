#pragma once

#include "CmptType.h"

namespace Ubpa::UECS {
	class Archetype;
	struct Chunk;

	class ChunkView {
	public:
		ChunkView(Archetype* archetype, size_t chunkIdx)
			: archetype{ archetype }, chunkIdx{ chunkIdx } {}

		bool Contains(CmptType) const;

		// nullptr if not contain
		void* GetCmptArray(CmptType) const;
		template<typename Cmpt>
		Cmpt* GetCmptArray() const { return reinterpret_cast<Cmpt*>(GetCmptArray(CmptType::Of<Cmpt>)); }
		const Entity* GetEntityArray() const { return GetCmptArray<Entity>(); }
		size_t EntityNum() const noexcept;

	private:
		Archetype* archetype;
		size_t chunkIdx;
	};
}
