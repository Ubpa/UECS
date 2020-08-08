#pragma once

#include "EntityQuery.h"
#include "SingletonLocator.h"
#include "Entity.h"
#include "CmptsView.h"
#include "SingletonsView.h"
#include "ChunkView.h"

#include <functional>

namespace Ubpa::UECS {
	// [description]
	// system function registered by Schedule in <System>::OnUpdate(Schedule&)
	// name + query + function<...>
	// name must be unique in global
	// query.filter can be change dynamically by other <System> with Schedule
	// [system function kind] (distinguish by argument list)
	// common : [World*], [{LastFrame|Write|Latest}Singleton<Component>], [SingletonsView]
	// 1. per entity function
	// * [Entity]
	// * [size_t indexInQuery]
	// * <tagged-component>: {LastFrame|Write|Latest}<Component>
	// * [CmptsView]
	// 2. chunk: ChunkView
	// 3. job
	class SystemFunc {
	public:
		enum class Mode {
			Entity,
			Chunk,
			Job,
		};

		EntityQuery entityQuery;
		SingletonLocator singletonLocator;

		template<typename Func>
		SystemFunc(Func&& func, std::string name, ArchetypeFilter archetypeFilter);
		
		const std::string& Name() const noexcept { return name; }

		static constexpr size_t HashCode(std::string_view name) { return hash_string(name); }

		size_t HashCode() const noexcept { return hashCode; }

		void operator()(World*, SingletonsView singletonsView, Entity e, size_t entityIndexInQuery, CmptsView cmptsView);
		void operator()(World*, SingletonsView singletonsView, ChunkView chunkView);
		void operator()(World*, SingletonsView singletonsView);

		Mode GetMode() const noexcept { return mode; }

		bool operator==(const SystemFunc& func) const noexcept { return name == func.name; }
	private:
		std::function<void(World*, SingletonsView singletonsView, Entity, size_t, CmptsView, ChunkView)> func;

		std::string name;
		Mode mode;
		size_t hashCode; // after name
	};
}

#include "detail/SystemFunc.inl"
