#pragma once

#include "EntityQuery.h"
#include "Entity.h"
#include "RTDCmptViewer.h"

#include <functional>

namespace Ubpa {
	// [description]
	// system function registered by Schedule in <System>::OnUpdate(Schedule&)
	// name + query + function<...>
	// name must be unique in global
	// query.filter can be change dynamically by other <System> with Schedule
	// [system function kind] (distinguish by argument list)
	// 1. per entity function: [[const] Entity e, ] [size_t indexInQuery, ] <Tagged_Component>...
	// - - tagged component: CmptTag::{LastFrame|Write|Latest}<Component>
	// 2. job: empty argument list
	// 3. runtime dynamic function: const EntityLocator* locator, void** cmpts
	class SystemFunc {
	public:
		EntityQuery query;
		
		template<typename Func>
		SystemFunc(Func&& func, std::string name, EntityFilter filter = EntityFilter{});

		// run-time dynamic function
		template<typename Func>
		SystemFunc(Func&& func, std::string name, EntityLocator locator, EntityFilter filter = EntityFilter{});
		
		const std::string& Name() const noexcept { return name; }

		static constexpr size_t HashCode(std::string_view name) { return hash_string(name); }

		size_t HashCode() const noexcept { return hashCode; }

		void operator()(Entity e, size_t entityIndexInQuery, RTDCmptViewer rtdcmpts) {
			return func(e, entityIndexInQuery, rtdcmpts);
		}

		// no arguments <Func>
		bool IsJob() const noexcept { return isJob; }

		bool operator==(const SystemFunc& func) const noexcept { return name == func.name; }
	private:
		template<typename Func, typename ArgList>
		SystemFunc(Func&& func, std::string name, EntityFilter filter, ArgList);

		std::function<void(Entity, size_t, RTDCmptViewer)> func;

		std::string name;
		bool isJob;
		size_t hashCode; // after name
	};
}

#include "detail/SystemFunc.inl"
