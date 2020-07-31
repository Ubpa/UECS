#include <UECS/ChunkView.h>

#include <UECS/detail/Archetype.h>

using namespace Ubpa::UECS;

void* ChunkView::GetCmptArray(CmptType t) const {
	return Contains(t) ? archetype->Locate(chunkIdx, t) : nullptr;
}

size_t ChunkView::EntityNum() const {
	return archetype->EntityNumOfChunk(chunkIdx);
}

bool ChunkView::Contains(CmptType t) const {
	return archetype->GetCmptTypeSet().Contains(t);
}
