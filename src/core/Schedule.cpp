#include <UECS/Schedule.h>

#include <UContainer/Algorithm.h>

using namespace Ubpa;
using namespace std;

Schedule& Schedule::Order(string_view x, string_view y) {
	sysFuncOrder.emplace(SystemFunc::HashCode(x), SystemFunc::HashCode(y));
	return *this;
}

Schedule& Schedule::InsertAll(string_view sys, CmptType type) {
	size_t hashcode = SystemFunc::HashCode(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseAlls.erase(type);
	change.insertAlls.insert(type);
	return *this;
}

Schedule& Schedule::InsertAny(string_view sys, CmptType type) {
	size_t hashcode = SystemFunc::HashCode(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseAnys.erase(type);
	change.insertAnys.insert(type);
	return *this;
}

Schedule& Schedule::InsertNone(string_view sys, CmptType type) {
	size_t hashcode = SystemFunc::HashCode(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseNones.erase(type);
	change.insertNones.insert(type);
	return *this;
}

Schedule& Schedule::EraseAll(string_view sys, CmptType type) {
	size_t hashcode = SystemFunc::HashCode(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseAlls.insert(type);
	change.insertAlls.erase(type);
	return *this;
}

Schedule& Schedule::EraseAny(string_view sys, CmptType type) {
	size_t hashcode = SystemFunc::HashCode(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseAnys.insert(type);
	change.insertAnys.erase(type);
	return *this;
}

Schedule& Schedule::EraseNone(string_view sys, CmptType type) {
	size_t hashcode = SystemFunc::HashCode(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseNones.insert(type);
	change.insertNones.erase(type);
	return *this;
}

void Schedule::Clear() {
	sysFuncs.clear();
	sysFuncOrder.clear();
}

Schedule::NoneGroup::NoneGroup(SystemFunc* func)
	: sysFuncs{func}
{
	allTypes = SetUnion(func->query.filter.AllCmptTypes(), func->query.filter.AnyCmptTypes());
	allTypes = SetUnion(allTypes, func->query.locator.CmptTypes());
	noneTypes = func->query.filter.NoneCmptTypes();
}

void Schedule::SetPrePostEdge(SysFuncGraph& graph,
	const std::vector<SystemFunc*>& preReaders,
	const std::vector<SystemFunc*>& writers,
	const std::vector<SystemFunc*>& postReaders)
{
	if (!preReaders.empty()) {
		NoneGroup preGroup(preReaders.front());
		for (size_t i = 1; i < preReaders.size(); i++) {
			NoneGroup group(preReaders[i]);
			preGroup.allTypes = SetIntersection(preGroup.allTypes, group.allTypes);
			preGroup.noneTypes = SetIntersection(preGroup.noneTypes, group.noneTypes);
		}

		for (const auto& w : writers) {
			NoneGroup wG(w);
			if (!SetIntersection(preGroup.allTypes, wG.noneTypes).empty()
				|| !SetIntersection(wG.allTypes, preGroup.noneTypes).empty())
				continue;
			for(auto preReader : preReaders)
				graph.AddEdge(preReader, w);
		}
	}

	if (!postReaders.empty()) {
		NoneGroup postGroup(postReaders.front());
		for (size_t i = 1; i < postReaders.size(); i++) {
			NoneGroup group(postReaders[i]);
			postGroup.allTypes = SetIntersection(postGroup.allTypes, group.allTypes);
			postGroup.noneTypes = SetIntersection(postGroup.noneTypes, group.noneTypes);
		}

		for (const auto& w : writers) {
			NoneGroup wG(w);
			if (!SetIntersection(postGroup.allTypes, wG.noneTypes).empty()
				|| !SetIntersection(wG.allTypes, postGroup.noneTypes).empty())
				continue;
			for (auto postReader : postReaders)
				graph.AddEdge(w, postReader);
		}
	}
}

vector<Schedule::NoneGroup> Schedule::GenSortNoneGroup(SysFuncGraph graph,
	const std::vector<SystemFunc*>& preReaders,
	const std::vector<SystemFunc*>& writers,
	const std::vector<SystemFunc*>& postReaders)
{
	NoneGroup preGroup, postGroup;

	if (!preReaders.empty()) {
		preGroup = NoneGroup(preReaders.front());
		for (size_t i = 1; i < preReaders.size(); i++) {
			NoneGroup group(preReaders[i]);
			preGroup.allTypes = SetIntersection(preGroup.allTypes, group.allTypes);
			preGroup.noneTypes = SetIntersection(preGroup.noneTypes, group.noneTypes);
		}
	}
	if (!postReaders.empty()) {
		postGroup = NoneGroup(postReaders.front());
		for (size_t i = 1; i < postReaders.size(); i++) {
			NoneGroup group(postReaders[i]);
			postGroup.allTypes = SetIntersection(postGroup.allTypes, group.allTypes);
			postGroup.noneTypes = SetIntersection(postGroup.noneTypes, group.noneTypes);
		}
	}

	vector<SystemFunc*> funcs;
	if (!preGroup.sysFuncs.empty())
		funcs.push_back(*preGroup.sysFuncs.begin());
	if (!postGroup.sysFuncs.empty())
		funcs.push_back(*postGroup.sysFuncs.begin());
	for (auto w : writers)
		funcs.push_back(w);

	auto subgraph = graph.SubGraph(funcs);
	auto [success, sortedFuncs] = subgraph.Toposort();
	assert(success);

	vector<NoneGroup> rst;

	for (auto func : sortedFuncs) {
		if (!preGroup.sysFuncs.empty() && func == *preGroup.sysFuncs.begin())
			rst.push_back(move(preGroup));
		else if (!postGroup.sysFuncs.empty() && func == *postGroup.sysFuncs.begin())
				rst.push_back(move(postGroup));
		else // writer
			rst.push_back(NoneGroup(func));
	}

	for (size_t i = 0; i < rst.size(); i++) {
		auto& gi = rst[i];
		for (size_t j = i + 1; j < rst.size(); j++) {
			const auto& gj = rst[j];
			if (SetIntersection(gi.noneTypes, gj.allTypes).empty()
				&& SetIntersection(gi.allTypes, gj.noneTypes).empty())
				continue;

			bool haveOrder = false;
			for (auto ifunc : gi.sysFuncs) {
				for (auto jfunc : gj.sysFuncs) {
					if (subgraph.HavePath(ifunc, jfunc)
						|| subgraph.HavePath(jfunc, ifunc)) {
						haveOrder = true;
						break;
					}
				}
				if (haveOrder) break;
			}
			if (haveOrder) continue;

			for (auto sysFunc : gj.sysFuncs)
				gi.sysFuncs.insert(sysFunc);

			gi.allTypes = SetIntersection(gi.allTypes, gj.allTypes);
			gi.noneTypes = SetIntersection(gi.noneTypes, gj.noneTypes);

			rst.erase(rst.begin() + j);
			j--;
		}
	}

	return rst;
}

SysFuncGraph Schedule::GenSysFuncGraph() const {
	// TODO : parallel

	// [change func Filter]
	for (const auto& [hashcode, change] : sysFilterChange) {
		auto target = sysFuncs.find(hashcode);
		if (target == sysFuncs.end())
			continue;

		auto func = target->second;

		func->query.filter.InsertAll(change.insertAlls);
		func->query.filter.InsertAny(change.insertAnys);
		func->query.filter.InsertNone(change.insertNones);
		func->query.filter.EraseAll(change.eraseAlls);
		func->query.filter.EraseAny(change.eraseAnys);
		func->query.filter.EraseNone(change.eraseNones);
	}

	// [gen cmptSysFuncsMap]
	struct CmptSysFuncs {
		vector<SystemFunc*> lastFrameSysFuncs;
		vector<SystemFunc*> writeSysFuncs;
		vector<SystemFunc*> latestSysFuncs;
	};
	unordered_map<CmptType, CmptSysFuncs> cmptSysFuncsMap;

	for (const auto& [hashcode, sysFunc] : sysFuncs) {
		const auto& locator = sysFunc->query.locator;
		for (const auto& type : locator.LastFrameCmptTypes())
			cmptSysFuncsMap[type].lastFrameSysFuncs.push_back(sysFunc);
		for (const auto& type : locator.WriteCmptTypes())
			cmptSysFuncsMap[type].writeSysFuncs.push_back(sysFunc);
		for (const auto& type : locator.LatestCmptTypes())
			cmptSysFuncsMap[type].latestSysFuncs.push_back(sysFunc);
	}

	// [gen graph]
	SysFuncGraph graph;

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
		SetPrePostEdge(graph,
			cmptSysFuncs.lastFrameSysFuncs,
			cmptSysFuncs.writeSysFuncs,
			cmptSysFuncs.latestSysFuncs);
	}

	// [order]
	for (const auto& [type, cmptSysFuncs] : cmptSysFuncsMap) {
		if (cmptSysFuncs.writeSysFuncs.empty())
			continue;

		auto sortedGroup = GenSortNoneGroup(graph,
			cmptSysFuncs.lastFrameSysFuncs,
			cmptSysFuncs.writeSysFuncs,
			cmptSysFuncs.latestSysFuncs);
		
		for (size_t i = 0; i < sortedGroup.size() - 1; i++) {
			const auto& gx = sortedGroup[i];
			const auto& gy = sortedGroup[i + 1];
			for (auto fx : gx.sysFuncs) {
				for (auto fy : gy.sysFuncs)
					graph.AddEdge(fx, fy);
			}
		}
	}

	return graph;
}
