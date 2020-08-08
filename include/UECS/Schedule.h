#pragma once

#include "SystemFunc.h"
#include "detail/SysFuncGraph.h"
#include "detail/Job.h"

#include <UContainer/Pool.h>

#include <map>

namespace Ubpa::UECS::detail::Schedule_ {
	struct Compiler;
}

namespace Ubpa::UECS {
	class EntityMngr;
	class SystemMngr;

	// [description]
	// system infomation record
	// - SystemFunc
	// - orders
	// - dynamic filter changes
	// [detail]
	// schedule will be clear at the beginning of the next World::Update()
	class Schedule {
	public:
		template<typename Func>
		const SystemFunc* Register(Func&& func, std::string name, ArchetypeFilter filter = ArchetypeFilter{});

		// run-time dynamic function
		template<typename Func>
		const SystemFunc* Register(Func&& func, std::string name, CmptLocator locator, ArchetypeFilter filter = ArchetypeFilter{});

		Schedule& LockFilter(std::string_view sys);

		Schedule& Order(std::string_view x, std::string_view y);

		Schedule& InsertAll(std::string_view sys, CmptType);
		Schedule& InsertAny(std::string_view sys, CmptType);
		Schedule& InsertNone(std::string_view sys, CmptType);
		Schedule& EraseAll(std::string_view sys, CmptType);
		Schedule& EraseAny(std::string_view sys, CmptType);
		Schedule& EraseNone(std::string_view sys, CmptType);

	private:
		template<typename... Args>
		const SystemFunc* Request(Args&&... args);

		void Clear();

		struct CmptSysFuncs {
			std::vector<SystemFunc*> lastFrameSysFuncs;
			std::vector<SystemFunc*> writeSysFuncs;
			std::vector<SystemFunc*> latestSysFuncs;
		};
		friend struct detail::Schedule_::Compiler;
		std::unordered_map<CmptType, CmptSysFuncs> GenCmptSysFuncsMap() const;

		SysFuncGraph GenSysFuncGraph() const;

		// SystemFunc's hashcode to pointer of SystemFunc
		std::unordered_map<size_t, SystemFunc*> sysFuncs;

		// SystemFunc's hashcode to SystemFunc's hashcode
		// parent to children
		std::unordered_map<size_t, size_t> sysFuncOrder;

		struct FilterChange {
			std::set<CmptType> insertAlls;
			std::set<CmptType> insertAnys;
			std::set<CmptType> insertNones;
			std::set<CmptType> eraseAlls;
			std::set<CmptType> eraseAnys;
			std::set<CmptType> eraseNones;
		};
		std::unordered_map<size_t, FilterChange> sysFilterChange;
		std::unordered_set<size_t> sysLockFilter;

		Pool<SystemFunc> sysFuncPool;
		friend class World;
	};
}

#include "detail/Schedule.inl"
