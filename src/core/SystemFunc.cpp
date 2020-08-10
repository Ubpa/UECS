#include <UECS/SystemFunc.h>

using namespace Ubpa::UECS;

void SystemFunc::operator()(World* w, SingletonsView singletonsView, Entity e, size_t entityIndexInQuery, CmptsView cmptsView) {
	assert(mode == Mode::Entity);
	return func(
		w,
		singletonsView,
		e,
		entityIndexInQuery,
		cmptsView,
		ChunkView{ nullptr, size_t_invalid }
	);
}

void SystemFunc::operator()(World* w, SingletonsView singletonsView, ChunkView chunkView) {
	assert(mode == Mode::Chunk);
	return func(
		w,
		singletonsView,
		Entity::Invalid(),
		size_t_invalid,
		CmptsView{ nullptr, 0 },
		chunkView
	);
}

void SystemFunc::operator()(World* w, SingletonsView singletonsView) {
	assert(mode == Mode::Job);
	return func(
		w,
		singletonsView,
		Entity::Invalid(),
		size_t_invalid,
		CmptsView{ nullptr, 0 },
		ChunkView{ nullptr, size_t_invalid }
	);
}
