#include <UECS/Schedule.h>

#include <UContainer/Algorithm.h>

#include <UECS/World.h>

using namespace Ubpa;
using namespace std;

namespace Ubpa::detail::Schedule_ {
	struct CmptSysFuncs {
		vector<SystemFunc*> lastFrameSysFuncs;
		vector<SystemFunc*> writeSysFuncs;
		vector<SystemFunc*> latestSysFuncs;
	};

	struct NoneGroup {
		NoneGroup() = default;
		NoneGroup(SystemFunc* func)
			: sysFuncs{ func }
		{
			needTypes = SetUnion(func->query.filter.AllCmptTypes(), func->query.filter.AnyCmptTypes());
			needTypes = SetUnion(needTypes, func->query.locator.CmptTypes());
			noneTypes = func->query.filter.NoneCmptTypes();
		}

		static bool Parallelable(const NoneGroup& x, const NoneGroup& y) {
			return !SetIntersection(x.needTypes, y.noneTypes).empty()
				|| !SetIntersection(y.needTypes, x.noneTypes).empty();
		}

		NoneGroup& operator+=(const NoneGroup& y) {
			needTypes = SetIntersection(needTypes, y.needTypes);
			noneTypes = SetIntersection(noneTypes, y.noneTypes);
			sysFuncs = SetUnion(sysFuncs, y.sysFuncs);
			return *this;
		}

		set<CmptType> needTypes;
		set<CmptType> noneTypes;
		set<SystemFunc*> sysFuncs;
	};

	void SetPrePostEdge(SysFuncGraph& graph,
		const CmptSysFuncs& cmptFuncs,
		const unordered_map<SystemFunc*, NoneGroup>& gMap)
	{
		const auto& preReaders = cmptFuncs.lastFrameSysFuncs;
		const auto& writers = cmptFuncs.writeSysFuncs;
		const auto& postReaders = cmptFuncs.latestSysFuncs;

		if (!preReaders.empty()) {
			NoneGroup preGroup{ gMap.find(preReaders.front())->second };
			for (size_t i = 1; i < preReaders.size(); i++)
				preGroup += gMap.find(preReaders[i])->second;

			for (const auto& w : writers) {
				if (NoneGroup::Parallelable(preGroup, gMap.find(w)->second))
					continue;
				for (auto preReader : preReaders)
					graph.AddEdge(preReader, w);
			}
		}

		if (!postReaders.empty()) {
			NoneGroup postGroup{ gMap.find(postReaders.front())->second };
			for (size_t i = 1; i < postReaders.size(); i++)
				postGroup += gMap.find(postReaders[i])->second;

			for (const auto& w : writers) {
				if (NoneGroup::Parallelable(postGroup, gMap.find(w)->second))
					continue;
				for (auto postReader : postReaders)
					graph.AddEdge(w, postReader);
			}
		}
	}

	vector<NoneGroup> GenSortNoneGroup(SysFuncGraph graph,
		const CmptSysFuncs& cmptFuncs,
		const unordered_map<SystemFunc*, NoneGroup>& gMap)
	{
		const auto& preReaders = cmptFuncs.lastFrameSysFuncs;
		const auto& writers = cmptFuncs.writeSysFuncs;
		const auto& postReaders = cmptFuncs.latestSysFuncs;

		NoneGroup preGroup, postGroup;

		if (!preReaders.empty()) {
			preGroup = gMap.find(preReaders.front())->second;
			for (size_t i = 1; i < preReaders.size(); i++)
				preGroup += gMap.find(preReaders[i])->second;
		}

		if (!postReaders.empty()) {
			postGroup = gMap.find(postReaders.front())->second;
			for (size_t i = 1; i < postReaders.size(); i++)
				postGroup += gMap.find(postReaders[i])->second;
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
				rst.push_back(gMap.find(func)->second);
		}

		for (size_t i = 0; i < rst.size(); i++) {
			auto& gi = rst[i];
			for (size_t j = i + 1; j < rst.size(); j++) {
				const auto& gj = rst[j];
				if (!NoneGroup::Parallelable(gi, gj))
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

				gi += gj;

				rst.erase(rst.begin() + j);
				j--;
			}
		}

		return rst;
	}
}

Schedule& Schedule::Order(string_view x, string_view y) {
	sysFuncOrder.emplace(SystemFunc::HashCode(x), SystemFunc::HashCode(y));
	return *this;
}

size_t Schedule::EntityNumInQuery(string_view sys) const {
	size_t hashcode = SystemFunc::HashCode(sys);
	auto target = sysFuncs.find(hashcode);
	if (target == sysFuncs.end())
		return size_t_invalid;

	const_cast<Schedule*>(this)->LockFilter(sys);

	auto func = target->second;
	return entityMngr->EntityNum(func->query);
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
	for (const auto& [hash, sysFunc] : sysFuncs)
		sysFuncPool.Recycle(sysFunc);
	sysFuncs.clear();
	sysFuncOrder.clear();
	sysFilterChange.clear();
	sysLockFilter.clear();
}

SysFuncGraph Schedule::GenSysFuncGraph() const {
	// TODO : parallel

	// [change func Filter]
	for (const auto& [hashcode, change] : sysFilterChange) {
		if (sysLockFilter.find(hashcode) != sysLockFilter.end())
			continue;

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
	
	unordered_map<CmptType, detail::Schedule_::CmptSysFuncs> cmptSysFuncsMap;

	for (const auto& [hashcode, sysFunc] : sysFuncs) {
		const auto& locator = sysFunc->query.locator;
		for (const auto& type : locator.LastFrameCmptTypes())
			cmptSysFuncsMap[type].lastFrameSysFuncs.push_back(sysFunc);
		for (const auto& type : locator.WriteCmptTypes())
			cmptSysFuncsMap[type].writeSysFuncs.push_back(sysFunc);
		for (const auto& type : locator.LatestCmptTypes())
			cmptSysFuncsMap[type].latestSysFuncs.push_back(sysFunc);
	}

	// [gen groupMap]
	unordered_map<SystemFunc*, detail::Schedule_::NoneGroup> groupMap;
	for (const auto& [hashcode, sysFunc] : sysFuncs)
		groupMap.emplace(sysFunc, sysFunc);

	// [gen graph]
	SysFuncGraph graph;

	// [gen graph] - vertex
	for (const auto& [hashcode, sysFunc] : sysFuncs)
		graph.AddVertex(sysFunc);
	
	// [gen graph] - edge - order
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

	// [gen graph] - edge - time point
	for (const auto& [type, cmptSysFuncs] : cmptSysFuncsMap)
		detail::Schedule_::SetPrePostEdge(graph, cmptSysFuncs, groupMap);

	// [gen graph] - edge - none group
	for (const auto& [type, cmptSysFuncs] : cmptSysFuncsMap) {
		if (cmptSysFuncs.writeSysFuncs.empty())
			continue;

		auto sortedGroup = detail::Schedule_::GenSortNoneGroup(graph, cmptSysFuncs, groupMap);
		
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
