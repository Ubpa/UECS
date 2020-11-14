#include <UECS/detail/SystemFunc.h>

using namespace Ubpa::UECS;

void SystemFunc::operator()(World* w, SingletonsView singletonsView, Entity e, size_t entityIndexInQuery, CmptsView cmptsView) const {
	assert(mode == Mode::Entity);
	return func(
		w,
		singletonsView,
		e,
		entityIndexInQuery,
		cmptsView,
		{}
	);
}

void SystemFunc::operator()(World* w, SingletonsView singletonsView, size_t entityBeginIndexInQuery, ChunkView chunkView) const {
	assert(mode == Mode::Chunk);
	return func(
		w,
		singletonsView,
		Entity::Invalid(),
		entityBeginIndexInQuery,
		{},
		chunkView
	);
}

void SystemFunc::operator()(World* w, SingletonsView singletonsView) const {
	assert(mode == Mode::Job);
	return func(
		w,
		singletonsView,
		Entity::Invalid(),
		static_cast<size_t>(-1),
		{},
		{}
	);
}
