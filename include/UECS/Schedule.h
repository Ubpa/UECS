#pragma once

#include "SystemFunc.h"
#include "detail/SysFuncGraph.h"
#include "detail/Job.h"

#include <UContainer/Pool.h>

namespace Ubpa {
	class World;

	class Schedule {
	public:
		Schedule& Order(size_t x, size_t y);
		Schedule& Order(std::string_view x, std::string_view y);
		Schedule& Order(SystemFunc* x, SystemFunc* y);

		template<typename... Args>
		SystemFunc* Request(Args&&... args) {
			SystemFunc* sysFunc = sysFuncPool.Request(std::forward<Args>(args)...);
			sysFuncs.emplace(sysFunc->HashCode(), sysFunc);
			return sysFunc;
		}

		World* GetWorld() const noexcept { return world; }

	private:
		Schedule(World* world) : world{ world } {}
		void Clear();
		SysFuncGraph GenSysFuncGraph() const;

		struct NoneGroup {
			std::set<CmptType> allTypes;
			std::set<CmptType> noneTypes;
			std::set<SystemFunc*> sysFuncs;
		};
		static std::vector<NoneGroup> GenSortNoneGroup(SysFuncGraph writerGraph,
			const std::vector<SystemFunc*>& preReaders,
			const std::vector<SystemFunc*>& writers,
			const std::vector<SystemFunc*>& postReaders);

		// SystemFunc's hashcode to pointer of SystemFunc
		std::unordered_map<size_t, SystemFunc*> sysFuncs;

		// SystemFunc's hashcode to SystemFunc's hashcode
		// parent to children
		std::unordered_map<size_t, size_t> sysFuncOrder;

		Pool<SystemFunc> sysFuncPool;
		World* world;
		friend class World;
	};
}
