#include <UECS/SystemFunc.h>

using namespace Ubpa::UECS;

void SystemFunc::operator()(Entity e, size_t entityIndexInQuery, CmptsView rtdcmpts) {
	assert(mode == Mode::Entity);
	return func(
		e,
		entityIndexInQuery,
		rtdcmpts,
		ChunkView{ nullptr, size_t_invalid, nullptr }
	);
}

void SystemFunc::operator()(ChunkView chunkView) {
	assert(mode == Mode::Chunk);
	return func(
		Entity::Invalid(),
		size_t_invalid,
		CmptsView{ nullptr, nullptr },
		chunkView
	);
}

void SystemFunc::operator()() {
	assert(mode == Mode::Job);
	return func(
		Entity::Invalid(),
		size_t_invalid,
		CmptsView{ nullptr, nullptr },
		ChunkView{ nullptr, size_t_invalid, nullptr }
	);
}

