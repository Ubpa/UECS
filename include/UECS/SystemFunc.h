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
	// name('s hashcode) must be unique in global
	// query.filter can be change dynamically by other <System> with Schedule
	// [system function kind] (distinguish by argument list)
	// common : World*, SingletonsView
	// 1. Mode::Entity: per entity function
	// * {LastFrame|Latest}<Singleton<Cmpt>>
	// * Entity
	// * size_t indexInQuery
	// * <tagged-components>: {LastFrame|Write|Latest}<Cmpt>...
	// * CmptsView
	// 2. Mode::Chunk
	// * {LastFrame|Latest}<Singleton<Cmpt>>
	// * ChunkView (necessary)
	// 3. Mode::Job
	// * {LastFrame|Write|Latest}<Singleton<Cmpt>>
	class SystemFunc {
	public:
		enum class Mode {
			Entity,
			Chunk,
			Job,
		};

		EntityQuery entityQuery;
		SingletonLocator singletonLocator;

		// Mode::Entity
		template<typename Func>
		SystemFunc(Func&&, std::string name, ArchetypeFilter, CmptLocator, SingletonLocator);

		// Mode::Chunk
		template<typename Func>
		SystemFunc(Func&&, std::string name, ArchetypeFilter, SingletonLocator);

		// Mode::Job
		template<typename Func>
		SystemFunc(Func&&, std::string name, SingletonLocator);
		
		const std::string& Name() const noexcept { return name; }

		static constexpr size_t HashCode(std::string_view name) { return hash_string(name); }

		size_t HashCode() const noexcept { return hashCode; }

		void operator()(World*, SingletonsView, Entity, size_t entityIndexInQuery, CmptsView);
		void operator()(World*, SingletonsView, ChunkView);
		void operator()(World*, SingletonsView);

		Mode GetMode() const noexcept { return mode; }

		bool operator==(const SystemFunc& sysFunc) const noexcept { return name == sysFunc.name; }
	private:
		std::function<void(World*, SingletonsView, Entity, size_t, CmptsView, ChunkView)> func;

		std::string name;
		Mode mode;
		size_t hashCode; // after name
	};
}

#include "detail/SystemFunc.inl"
