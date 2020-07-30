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
		Schedule& Register(Func&& func, std::string name, EntityFilter filter = EntityFilter{});

		// run-time dynamic function
		template<typename Func>
		Schedule& Register(Func&& func, std::string name, EntityLocator locator, EntityFilter filter = EntityFilter{});

		Schedule& LockFilter(std::string_view sys);

		// if sys is unregistered, return size_t_invalid
		// call LockFilterChange(std::string_view)
		size_t EntityNumInQuery(std::string_view sys) const;

		EntityMngr* GetEntityMngr() const noexcept { return entityMngr; }
		SystemMngr* GetSystemMngr() const noexcept { return systemMngr; }

		Schedule& Order(std::string_view x, std::string_view y);

		Schedule& InsertAll(std::string_view sys, CmptType);
		Schedule& InsertAny(std::string_view sys, CmptType);
		Schedule& InsertNone(std::string_view sys, CmptType);
		Schedule& EraseAll(std::string_view sys, CmptType);
		Schedule& EraseAny(std::string_view sys, CmptType);
		Schedule& EraseNone(std::string_view sys, CmptType);
		template<typename Cmpt> Schedule& InsertAll(std::string_view sys) { return InsertAll(sys, CmptType::Of<Cmpt>); }
		template<typename Cmpt> Schedule& InsertAny(std::string_view sys) { return InsertAny(sys, CmptType::Of<Cmpt>); }
		template<typename Cmpt> Schedule& InsertNone(std::string_view sys) { return InsertNone(sys, CmptType::Of<Cmpt>); }
		template<typename Cmpt> Schedule& EraseAll(std::string_view sys) { return EraseAll(sys, CmptType::Of<Cmpt>); }
		template<typename Cmpt> Schedule& EraseAny(std::string_view sys) { return EraseAny(sys, CmptType::Of<Cmpt>); }
		template<typename Cmpt> Schedule& EraseNone(std::string_view sys) { return EraseNone(sys, CmptType::Of<Cmpt>); }

	private:
		template<typename... Args>
		void Request(Args&&... args);

		Schedule(EntityMngr* entityMngr, SystemMngr* systemMngr);
		void Clear();

		struct CmptSysFuncs {
			std::vector<SystemFunc*> lastFrameSysFuncs;
			std::vector<SystemFunc*> writeSysFuncs;
			std::vector<SystemFunc*> latestSysFuncs;
		};
		friend struct detail::Schedule_::Compiler;
		std::unordered_map<CmptType, CmptSysFuncs> cmptSysFuncsMap;

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
		EntityMngr* entityMngr;
		SystemMngr* systemMngr;
		friend class World;
	};
}

#include "detail/Schedule.inl"
