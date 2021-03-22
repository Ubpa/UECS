#include <UECS/SystemFunc.hpp>

using namespace Ubpa::UECS;

void SystemFunc::operator()(World* w, SingletonsView singletonsView, Entity e, std::size_t entityIndexInQuery, CmptsView cmptsView, CommandBufferView cbv) const {
	assert(mode == Mode::Entity);
	return func(
		w,
		singletonsView,
		e,
		entityIndexInQuery,
		cmptsView,
		{},
		cbv
	);
}

void SystemFunc::operator()(World* w, SingletonsView singletonsView, std::size_t entityBeginIndexInQuery, ChunkView chunkView, CommandBufferView cbv) const {
	assert(mode == Mode::Chunk);
	return func(
		w,
		singletonsView,
		Entity::Invalid(),
		entityBeginIndexInQuery,
		{},
		chunkView,
		cbv
	);
}

void SystemFunc::operator()(World* w, SingletonsView singletonsView) const {
	assert(mode == Mode::Job);
	return func(
		w,
		singletonsView,
		Entity::Invalid(),
		static_cast<std::size_t>(-1),
		{},
		{},
		{}
	);
}
