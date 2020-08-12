#include <UECS/ChunkView.h>

#include <UECS/detail/Archetype.h>

using namespace Ubpa::UECS;

void* ChunkView::GetCmptArray(CmptType t) const {
	return archetype->Locate(chunkIdx, t);
}

size_t ChunkView::EntityNum() const noexcept {
	return archetype->EntityNumOfChunk(chunkIdx);
}

bool ChunkView::Contains(CmptType t) const {
	return archetype->GetCmptTypeSet().Contains(t);
}
