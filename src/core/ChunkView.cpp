#include <UECS/ChunkView.h>

#include <UECS/detail/Archetype.h>

using namespace Ubpa::UECS;

ChunkView::ChunkView(Archetype* archetype, size_t chunkIdx) noexcept
	: archetype{ archetype }, chunkIdx{ chunkIdx }, entityNum{ archetype->EntityNumOfChunk(chunkIdx) } {}

void* ChunkView::GetCmptArray(CmptType t) const {
	return archetype->Locate(chunkIdx, t);
}

bool ChunkView::Contains(CmptType t) const {
	return archetype->GetCmptTypeSet().Contains(t);
}
