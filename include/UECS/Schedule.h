#pragma once

#include "SystemFunc.h"
#include "detail/SysFuncGraph.h"
#include "detail/Job.h"

#include <UContainer/Pool.h>

#include <map>

namespace Ubpa {
	class World;

	class Schedule {
	public:
		template<typename... Args>
		Schedule& Request(Args&&... args) {
			SystemFunc* sysFunc = sysFuncPool.Request(std::forward<Args>(args)...);
			sysFuncs.emplace(sysFunc->HashCode(), sysFunc);
			return *this;
		}

		World* GetWorld() const noexcept { return world; }

		Schedule& Order(std::string_view x, std::string_view y);

		Schedule& InsertAll(std::string_view sys, CmptType);
		Schedule& InsertAny(std::string_view sys, CmptType);
		Schedule& InsertNone(std::string_view sys, CmptType);
		Schedule& EraseAll(std::string_view sys, CmptType);
		Schedule& EraseAny(std::string_view sys, CmptType);
		Schedule& EraseNone(std::string_view sys, CmptType);
		template<typename Cmpt> Schedule& InsertAll(std::string_view sys) { return InsertAll(sys,CmptType::Of<Cmpt>()); }
		template<typename Cmpt> Schedule& InsertAny(std::string_view sys) { return InsertAny(sys, CmptType::Of<Cmpt>()); }
		template<typename Cmpt> Schedule& InsertNone(std::string_view sys) { return InsertNone(sys, CmptType::Of<Cmpt>()); }
		template<typename Cmpt> Schedule& EraseAll(std::string_view sys) { return EraseAll(sys, CmptType::Of<Cmpt>()); }
		template<typename Cmpt> Schedule& EraseAny(std::string_view sys) { return EraseAny(sys, CmptType::Of<Cmpt>()); }
		template<typename Cmpt> Schedule& EraseNone(std::string_view sys) { return EraseNone(sys, CmptType::Of<Cmpt>()); }

	private:
		Schedule(World* world) : world{ world } {}
		void Clear();
		SysFuncGraph GenSysFuncGraph() const;

		struct NoneGroup {
			NoneGroup() = default;
			NoneGroup(SystemFunc* func);

			std::set<CmptType> allTypes;
			std::set<CmptType> noneTypes;
			std::set<SystemFunc*> sysFuncs;
		};

		static void SetPrePostEdge(SysFuncGraph& graph,
			const std::vector<SystemFunc*>& preReaders,
			const std::vector<SystemFunc*>& writers,
			const std::vector<SystemFunc*>& postReaders);

		static std::vector<NoneGroup> GenSortNoneGroup(SysFuncGraph graph,
			const std::vector<SystemFunc*>& preReaders,
			const std::vector<SystemFunc*>& writers,
			const std::vector<SystemFunc*>& postReaders);

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

		Pool<SystemFunc> sysFuncPool;
		World* world;
		friend class World;
	};
}
