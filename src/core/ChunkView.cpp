#include <UECS/ChunkView.h>

#include "Archetype.h"

using namespace Ubpa::UECS;

ChunkView::ChunkView(Archetype* archetype, std::size_t chunkIdx) noexcept
	: archetype{ archetype }, chunkIdx{ chunkIdx }, entityNum{ archetype->EntityNumOfChunk(chunkIdx) } {}

void* ChunkView::GetCmptArray(TypeID t) const {
	return archetype->Locate(chunkIdx, t);
}

bool ChunkView::Contains(TypeID t) const {
	return archetype->GetTypeIDSet().Contains(t);
}
