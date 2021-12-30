#include <UECS/SystemFunc.hpp>

using namespace Ubpa::UECS;

std::string_view SystemFunc::Name() const noexcept { return name; }

std::size_t SystemFunc::GetValue() const noexcept { return hashCode; }

void SystemFunc::operator()(World* w, SingletonsView singletonsView, Entity e, std::size_t entityIndexInQuery, CmptsView cmptsView, CommandBufferPtr cb) const {
	assert(mode == Mode::Entity);
	return func(
		w,
		singletonsView,
		e,
		entityIndexInQuery,
		cmptsView,
		{},
		cb
	);
}

void SystemFunc::operator()(World* w, SingletonsView singletonsView, std::size_t entityBeginIndexInQuery, ChunkView chunkView, CommandBufferPtr cb) const {
	assert(mode == Mode::Chunk);
	return func(
		w,
		singletonsView,
		Entity::Invalid(),
		entityBeginIndexInQuery,
		{},
		chunkView,
		cb
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


SystemFunc::Mode SystemFunc::GetMode() const noexcept { return mode; }
bool SystemFunc::IsParallel() const noexcept { return isParallel; }

bool SystemFunc::operator==(const SystemFunc& sysFunc) const noexcept { return name == sysFunc.name; }
