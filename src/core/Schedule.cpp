#include <UECS/Schedule.h>

using namespace Ubpa;
using namespace std;

Schedule& Schedule::Order(size_t x, size_t y) {
	sysFuncOrder.emplace(x, y);
	return *this;
}

Schedule& Schedule::Order(std::string_view x, std::string_view y) {
	return Order(SystemFunc::HashCode(x), SystemFunc::HashCode(y));
}

Schedule& Schedule::Order(SystemFunc* x, SystemFunc* y) {
	assert(x != nullptr && y != nullptr);
	return Order(x->HashCode(), y->HashCode());
}

void Schedule::Clear() {
	sysFuncs.clear();
	sysFuncOrder.clear();
}

SysFuncGraph Schedule::GenSysFuncGraph() const {
	// TODO : parallel

	SysFuncGraph graph;

	// SystemFuncs related with <Cmpt>
	struct CmptSysFuncs {
		std::vector<SystemFunc*> lastFrameSysFuncs;
		std::vector<SystemFunc*> writeSysFuncs;
		std::vector<SystemFunc*> latestSysFuncs;
	};
	std::unordered_map<CmptType, CmptSysFuncs> cmptSysFuncsMap;

	for (const auto& [hashcode, sysFunc] : sysFuncs) {
		const auto& locator = sysFunc->query.Locator();
		for (const auto& type : locator.LastFrameCmptTypes())
			cmptSysFuncsMap[type].lastFrameSysFuncs.push_back(sysFunc);
		for (const auto& type : locator.WriteCmptTypes())
			cmptSysFuncsMap[type].writeSysFuncs.push_back(sysFunc);
		for (const auto& type : locator.LatestCmptTypes())
			cmptSysFuncsMap[type].latestSysFuncs.push_back(sysFunc);
	}

	for (const auto& [hashcode, sysFunc] : sysFuncs)
		graph.AddVertex(sysFunc);

	for (const auto& [x, y] : sysFuncOrder) {
		auto target_x = sysFuncs.find(x);
		if (target_x == sysFuncs.end())
			continue;
		auto target_y = sysFuncs.find(y);
		if (target_y == sysFuncs.end())
			continue;

		auto sysFunc_x = target_x->second;
		auto sysFunc_y = target_y->second;
		graph.AddEdge(sysFunc_x, sysFunc_y);
	}

	for (const auto& [type, cmptSysFuncs] : cmptSysFuncsMap) {
		if (cmptSysFuncs.writeSysFuncs.empty())
			continue;

		auto subgraph = graph.SubGraph(cmptSysFuncs.writeSysFuncs);
		auto [success, sortedWriteSysFuncs] = subgraph.Toposort();
		assert(success);
		
		for (const auto& lastFrameSysFunc : cmptSysFuncs.lastFrameSysFuncs)
			graph.AddEdge(lastFrameSysFunc, sortedWriteSysFuncs.front());
		for (const auto& latestSysFunc : cmptSysFuncs.latestSysFuncs)
			graph.AddEdge(sortedWriteSysFuncs.back(), latestSysFunc);
		for (size_t i = 0; i < sortedWriteSysFuncs.size() - 1; i++)
			graph.AddEdge(sortedWriteSysFuncs[i], sortedWriteSysFuncs[i + 1]);
	}

	return graph;
}
