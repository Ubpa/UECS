#include <UECS/ChunkView.hpp>

#include "Archetype.hpp"

using namespace Ubpa::UECS;

ChunkView::ChunkView(Archetype* archetype, std::size_t chunkIdx) noexcept
	: archetype{ archetype }, chunkIdx{ chunkIdx }, entityNum{ archetype->EntityNumOfChunk(chunkIdx) } {}

void* ChunkView::GetCmptArray(TypeID t) const {
	return archetype->Locate(chunkIdx, t);
}

bool ChunkView::Contains(TypeID t) const {
	return archetype->GetTypeIDSet().Contains(t);
}
