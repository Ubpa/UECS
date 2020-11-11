#include <UECS/Schedule.h>

#include <UContainer/Algorithm.h>

#include "SysFuncGraph.h"

using namespace Ubpa::UECS;
using namespace std;

namespace Ubpa::UECS::detail {
	struct NoneGroup {
		NoneGroup() = default;
		NoneGroup(SystemFunc* func)
			: sysFuncs{ func }
		{
			allTypes = SetUnion(func->entityQuery.filter.all, func->entityQuery.locator.CmptAccessTypes());
			anyTypes = func->entityQuery.filter.any;
			randomTypes = func->randomAccessor.types;
			noneTypes = func->entityQuery.filter.none;
		}

		static bool Parallelable(const NoneGroup& x, const NoneGroup& y) {
			//{ // subgraph
			//	const NoneGroup& minG = x.sysFuncs.size() < y.sysFuncs.size() ? x : y;
			//	const NoneGroup& maxG = x.sysFuncs.size() < y.sysFuncs.size() ? y : x;
			//	bool flag = true;
			//	for(auto* f : minG.sysFuncs) {
			//		if(maxG.sysFuncs.find(f) == maxG.sysFuncs.end()) {
			//			flag = false;
			//			break;
			//		}
			//	}
			//	if (flag)
			//		return true;
			//}
			if (y.randomTypes.empty()) { // y.none
				bool allFlag = false;
				bool anyFlag = true;
				bool randomFlag = true;

				{ // all
					auto x_iter = x.allTypes.begin();
					auto y_iter = y.noneTypes.begin();
					while (x_iter != x.allTypes.end() && y_iter != y.noneTypes.end()) {
						if (x_iter->GetCmptType() < *y_iter)
							++x_iter;
						else if (x_iter->GetCmptType() > * y_iter)
							++y_iter;
						else { // == 
							allFlag = true;
							break;
						}
					}
				}
				if (!allFlag) {
					if (x.anyTypes.empty())
						anyFlag = false;
					else {
						// any
						auto x_iter = x.anyTypes.begin();
						auto y_iter = y.noneTypes.begin();
						while (x_iter != x.anyTypes.end() && y_iter != y.noneTypes.end()) {
							if (x_iter->GetCmptType() < *y_iter) {
								anyFlag = false;
								break;
							}

							if (x_iter->GetCmptType() == *y_iter)
								++x_iter;

							++y_iter;
						}
					}
				}
				if (allFlag || anyFlag) {
					// random
					auto x_iter = x.randomTypes.begin();
					auto y_iter = y.noneTypes.begin();
					while (x_iter != x.randomTypes.end() && y_iter != y.noneTypes.end()) {
						if (x_iter->GetCmptType() < *y_iter) {
							randomFlag = false;
							break;
						}

						if (x_iter->GetCmptType() == *y_iter)
							++x_iter;

						++y_iter;
					}
					if (randomFlag)
						return true;
				}
			}
			if (x.randomTypes.empty()) { // x.none
				bool allFlag = false;
				bool anyFlag = true;
				bool randomFlag = true;
				{ // all
					auto x_iter = x.noneTypes.begin();
					auto y_iter = y.allTypes.begin();
					while (x_iter != x.noneTypes.end() && y_iter != y.allTypes.end()) {
						if (*x_iter < y_iter->GetCmptType())
							++x_iter;
						else if (*x_iter > y_iter->GetCmptType())
							++y_iter;
						else { // == 
							allFlag = true;
							break;
						}
					}
				}
				if (!allFlag) {
					// any
					if (y.anyTypes.empty())
						anyFlag = false;
					else {
						auto x_iter = x.noneTypes.begin();
						auto y_iter = y.anyTypes.begin();
						while (x_iter != x.noneTypes.end() && y_iter != y.anyTypes.end()) {
							if (y_iter->GetCmptType() < *x_iter) {
								anyFlag = false;
								break;
							}

							if (y_iter->GetCmptType() == *x_iter)
								++y_iter;

							++x_iter;
						}
					}
				}
				if (allFlag || anyFlag) {
					// random
					auto x_iter = x.noneTypes.begin();
					auto y_iter = y.randomTypes.begin();
					while (x_iter != x.noneTypes.end() && y_iter != y.randomTypes.end()) {
						if (y_iter->GetCmptType() < *x_iter) {
							randomFlag = false;
							break;
						}

						if (y_iter->GetCmptType() == *x_iter)
							++y_iter;

						++x_iter;
					}
					if (randomFlag)
						return true;
				}
			}
			return false;
		}

		NoneGroup& operator+=(const NoneGroup& y) {
			allTypes = SetIntersection(allTypes, y.allTypes);
			anyTypes = SetUnion(anyTypes, y.anyTypes);
			anyTypes = SetUnion(anyTypes, SetSymmetricDifference(allTypes, y.allTypes));
			randomTypes = SetUnion(randomTypes, y.randomTypes);
			noneTypes = SetIntersection(noneTypes, y.noneTypes);
			sysFuncs = SetUnion(sysFuncs, y.sysFuncs);
			return *this;
		}

		CmptAccessTypeSet allTypes;
		CmptAccessTypeSet anyTypes;
		CmptAccessTypeSet randomTypes;
		set<CmptType> noneTypes;
		set<SystemFunc*> sysFuncs;
	};

	struct Compiler {
		static void SetPrePostEdge(SysFuncGraph& graph,
			const Schedule::CmptSysFuncs& cmptFuncs,
			const unordered_map<SystemFunc*, NoneGroup>& gMap)
		{
			const auto& preReaders = cmptFuncs.lastFrameSysFuncs;
			const auto& writers = cmptFuncs.writeSysFuncs;
			const auto& postReaders = cmptFuncs.latestSysFuncs;

			if (writers.empty())
				return;

			if (!preReaders.empty()) {
				NoneGroup preGroup{ gMap.at(preReaders.front()) };
				for (size_t i = 1; i < preReaders.size(); i++)
					preGroup += gMap.at(preReaders[i]);

				for (const auto& w : writers) {
					if (NoneGroup::Parallelable(preGroup, gMap.at(w)))
						continue;
					for (auto* preReader : preReaders)
						graph.AddEdge(preReader, w);
				}
			}

			if (!postReaders.empty()) {
				NoneGroup postGroup{ gMap.at(postReaders.front()) };
				for (size_t i = 1; i < postReaders.size(); i++)
					postGroup += gMap.at(postReaders[i]);

				for (const auto& w : writers) {
					if (NoneGroup::Parallelable(postGroup, gMap.at(w)))
						continue;
					for (auto* postReader : postReaders)
						graph.AddEdge(w, postReader);
				}
			}
		}

		static vector<NoneGroup> GenSortNoneGroup(
			const SysFuncGraph& graph,
			const Schedule::CmptSysFuncs& cmptFuncs,
			const unordered_map<SystemFunc*, NoneGroup>& gMap)
		{
			const auto& preReaders = cmptFuncs.lastFrameSysFuncs;
			const auto& writers = cmptFuncs.writeSysFuncs;
			const auto& postReaders = cmptFuncs.latestSysFuncs;

			NoneGroup preGroup, postGroup;

			if (!preReaders.empty()) {
				preGroup = gMap.at(preReaders.front());
				for (size_t i = 1; i < preReaders.size(); i++)
					preGroup += gMap.at(preReaders[i]);
			}

			if (!postReaders.empty()) {
				postGroup = gMap.at(postReaders.front());
				for (size_t i = 1; i < postReaders.size(); i++)
					postGroup += gMap.at(postReaders[i]);
			}

			vector<SystemFunc*> funcs;
			if (!preGroup.sysFuncs.empty())
				funcs.push_back(*preGroup.sysFuncs.begin());
			if (!postGroup.sysFuncs.empty())
				funcs.push_back(*postGroup.sysFuncs.begin());
			for (auto* w : writers)
				funcs.push_back(w);

			auto subgraph = graph.SubGraph(funcs);
			auto [success, sortedFuncs] = subgraph.Toposort();
			assert(success);

			vector<NoneGroup> rst;

			for (auto* func : sortedFuncs) {
				if (!preGroup.sysFuncs.empty() && func == *preGroup.sysFuncs.begin())
					rst.push_back(move(preGroup));
				else if (!postGroup.sysFuncs.empty() && func == *postGroup.sysFuncs.begin())
					rst.push_back(move(postGroup));
				else // writer
					rst.push_back(gMap.at(func));
			}

			for (size_t i = 0; i < rst.size(); i++) {
				auto& gi = rst[i];
				for (size_t j = i + 1; j < rst.size(); j++) {
					const auto& gj = rst[j];
					if (!NoneGroup::Parallelable(gi, gj))
						continue;

					bool haveOrder = false;
					for (auto* ifunc : gi.sysFuncs) {
						for (auto* jfunc : gj.sysFuncs) {
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
	};
}

Schedule& Schedule::Order(string_view x, string_view y) {
	sysFuncOrder.emplace(SystemFunc::HashCode(x), SystemFunc::HashCode(y));
	return *this;
}

Schedule& Schedule::InsertNone(string_view sys, CmptType type) {
	size_t hashcode = SystemFunc::HashCode(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseNones.erase(type);
	change.insertNones.insert(type);
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

unordered_map<CmptType, Schedule::CmptSysFuncs> Schedule::GenCmptSysFuncsMap() const {
	unordered_map<CmptType, CmptSysFuncs> rst;
	for (const auto& [hashcode, sysFunc] : sysFuncs) {
		for (const auto& type : sysFunc->entityQuery.locator.CmptAccessTypes()) {
			switch (type.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				rst[type].lastFrameSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::WRITE:
				rst[type].writeSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::LATEST:
				rst[type].latestSysFuncs.push_back(sysFunc);
				break;
			default:
				assert(false);
				break;
			}
		}
		for (const auto& type : sysFunc->singletonLocator.SingletonTypes()) {
			switch (type.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				rst[type].lastFrameSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::WRITE:
				rst[type].writeSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::LATEST:
				rst[type].latestSysFuncs.push_back(sysFunc);
				break;
			default:
				assert(false);
				break;
			}
		}
		for (const auto& type : sysFunc->randomAccessor.types) {
			switch (type.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				rst[type].lastFrameSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::WRITE:
				rst[type].writeSysFuncs.push_back(sysFunc);
				break;
			case AccessMode::LATEST:
				rst[type].latestSysFuncs.push_back(sysFunc);
				break;
			default:
				assert(false);
				break;
			}
		}

		if (sysFunc->GetMode() == SystemFunc::Mode::Chunk) {
			const auto& filter = sysFunc->entityQuery.filter;
			for (const auto& type : filter.all) {
				auto& cmptSysFuncs = rst[type];
				switch (type.GetAccessMode())
				{
				case AccessMode::LAST_FRAME:
					cmptSysFuncs.lastFrameSysFuncs.push_back(sysFunc);
					break;
				case AccessMode::WRITE:
					cmptSysFuncs.writeSysFuncs.push_back(sysFunc);
					break;
				case AccessMode::LATEST:
					cmptSysFuncs.latestSysFuncs.push_back(sysFunc);
					break;
				default:
					assert(false);
					break;
				}
			}

			for (const auto& type : filter.any) {
				auto& cmptSysFuncs = rst[type];
				switch (type.GetAccessMode())
				{
				case AccessMode::LAST_FRAME:
					cmptSysFuncs.lastFrameSysFuncs.push_back(sysFunc);
					break;
				case AccessMode::WRITE:
					cmptSysFuncs.writeSysFuncs.push_back(sysFunc);
					break;
				case AccessMode::LATEST:
					cmptSysFuncs.latestSysFuncs.push_back(sysFunc);
					break;
				default:
					assert(false);
					break;
				}
			}
		}
	}
	return rst;
}

SysFuncGraph Schedule::GenSysFuncGraph() const {
	// [change func Filter]
	for (const auto& [hashcode, change] : sysFilterChange) {
		if (sysLockFilter.find(hashcode) != sysLockFilter.end())
			continue;

		auto target = sysFuncs.find(hashcode);
		if (target == sysFuncs.end())
			continue;

		auto* func = target->second;

		for (const auto& type : change.insertNones)
			func->entityQuery.filter.none.insert(type);
		for (const auto& type : change.eraseNones)
			func->entityQuery.filter.none.erase(type);
	}

	auto cmptSysFuncsMap = GenCmptSysFuncsMap();

	// [gen groupMap]
	unordered_map<SystemFunc*, detail::NoneGroup> groupMap;
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

		auto* sysFunc_x = target_x->second;
		auto* sysFunc_y = target_y->second;
		graph.AddEdge(sysFunc_x, sysFunc_y);
	}

	// [gen graph] - edge - time point
	for (const auto& [type, cmptSysFuncs] : cmptSysFuncsMap)
		detail::Compiler::SetPrePostEdge(graph, cmptSysFuncs, groupMap);

	// [gen graph] - edge - none group
	for (const auto& [type, cmptSysFuncs] : cmptSysFuncsMap) {
		if (cmptSysFuncs.writeSysFuncs.empty())
			continue;

		auto sortedGroup = detail::Compiler::GenSortNoneGroup(graph, cmptSysFuncs, groupMap);
		
		for (size_t i = 0; i < sortedGroup.size() - 1; i++) {
			const auto& gx = sortedGroup[i];
			const auto& gy = sortedGroup[i + 1];
			for (auto* fx : gx.sysFuncs) {
				for (auto* fy : gy.sysFuncs)
					graph.AddEdge(fx, fy);
			}
		}
	}

	return graph;
}
