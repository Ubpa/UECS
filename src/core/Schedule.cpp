#include <UECS/Schedule.h>

#include "SysFuncGraph.h"

using namespace Ubpa;
using namespace Ubpa::UECS;
using namespace std;

namespace Ubpa::UECS::details {
	struct NoneGroup {
		NoneGroup() = default;
		NoneGroup(SystemFunc* func)
			: sysFuncs{ func }
		{
			std::set_union(
				func->entityQuery.filter.all.begin(),
				func->entityQuery.filter.all.end(),
				func->entityQuery.locator.AccessTypeIDs().begin(),
				func->entityQuery.locator.AccessTypeIDs().end(),
				std::insert_iterator<AccessTypeIDSet>(allTypes, allTypes.begin())
			);

			anyTypes = func->entityQuery.filter.any;
			randomTypes = func->randomAccessor.types;
			noneTypes = func->entityQuery.filter.none;
		}

		static bool Parallelable(const NoneGroup& x, const NoneGroup& y) {
			if (y.randomTypes.empty() && !y.noneTypes.empty()) { // y.none
				bool allFlag = false;
				bool anyFlag = true;
				bool randomFlag = true;

				{ // all
					auto x_iter = x.allTypes.begin();
					auto y_iter = y.noneTypes.begin();
					while (x_iter != x.allTypes.end() && y_iter != y.noneTypes.end()) {
						if (*x_iter < *y_iter)
							++x_iter;
						else if (*x_iter > * y_iter)
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
						while (x_iter != x.anyTypes.end()) {
							if (y_iter == y.noneTypes.end() || *x_iter < *y_iter) {
								anyFlag = false;
								break;
							}

							if (*x_iter == *y_iter)
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
						if (*x_iter < *y_iter) {
							randomFlag = false;
							break;
						}

						if (*x_iter == *y_iter)
							++x_iter;

						++y_iter;
					}
					if (randomFlag)
						return true;
				}
			}
			if (x.randomTypes.empty() && !x.noneTypes.empty()) { // x.none
				bool allFlag = false;
				bool anyFlag = true;
				bool randomFlag = true;
				{ // all
					auto x_iter = x.noneTypes.begin();
					auto y_iter = y.allTypes.begin();
					while (x_iter != x.noneTypes.end() && y_iter != y.allTypes.end()) {
						if (*x_iter < *y_iter)
							++x_iter;
						else if (*x_iter > *y_iter)
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
						while (x_iter != x.noneTypes.end()) {
							if (y_iter == y.anyTypes.end() || *y_iter < *x_iter) {
								anyFlag = false;
								break;
							}

							if (*y_iter == *x_iter)
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
						if (*y_iter < *x_iter) {
							randomFlag = false;
							break;
						}

						if (*y_iter == *x_iter)
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
			AccessTypeIDSet rst_allTypes, rst_anyTypes, rst_randomTypes,
				union_anyTypes, diff_allTypes;
			set<TypeID> rst_noneTypes;
			set<SystemFunc*> rst_sysFuncs;
			std::set_intersection(
				allTypes.begin(),
				allTypes.end(),
				y.allTypes.begin(),
				y.allTypes.end(),
				std::insert_iterator<AccessTypeIDSet>(rst_allTypes, rst_allTypes.begin())
			);
			std::set_union(
				anyTypes.begin(),
				anyTypes.end(),
				y.anyTypes.begin(),
				y.anyTypes.end(),
				std::insert_iterator<AccessTypeIDSet>(union_anyTypes, union_anyTypes.begin())
			);
			std::set_symmetric_difference(
				allTypes.begin(),
				allTypes.end(),
				y.allTypes.begin(),
				y.allTypes.end(),
				std::insert_iterator<AccessTypeIDSet>(diff_allTypes, diff_allTypes.begin())
			);
			std::set_union(
				union_anyTypes.begin(),
				union_anyTypes.end(),
				diff_allTypes.begin(),
				diff_allTypes.end(),
				std::insert_iterator<AccessTypeIDSet>(rst_anyTypes, rst_anyTypes.begin())
			);
			std::set_union(
				randomTypes.begin(),
				randomTypes.end(),
				y.randomTypes.begin(),
				y.randomTypes.end(),
				std::insert_iterator<AccessTypeIDSet>(rst_randomTypes, rst_randomTypes.begin())
			);
			std::set_intersection(
				noneTypes.begin(),
				noneTypes.end(),
				y.noneTypes.begin(),
				y.noneTypes.end(),
				std::insert_iterator<set<TypeID>>(rst_noneTypes, rst_noneTypes.begin())
			);
			std::set_union(
				sysFuncs.begin(),
				sysFuncs.end(),
				y.sysFuncs.begin(),
				y.sysFuncs.end(),
				std::insert_iterator<set<SystemFunc*>>(rst_sysFuncs, rst_sysFuncs.begin())
			);

			allTypes = std::move(rst_allTypes);
			anyTypes = std::move(rst_anyTypes);
			randomTypes = std::move(rst_randomTypes);
			noneTypes = std::move(rst_noneTypes);
			sysFuncs = std::move(rst_sysFuncs);
			return *this;
		}

		AccessTypeIDSet allTypes;
		AccessTypeIDSet anyTypes;
		AccessTypeIDSet randomTypes;
		set<TypeID> noneTypes;
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
				for (std::size_t i = 1; i < preReaders.size(); i++)
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
				for (std::size_t i = 1; i < postReaders.size(); i++)
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
				for (std::size_t i = 1; i < preReaders.size(); i++)
					preGroup += gMap.at(preReaders[i]);
			}

			if (!postReaders.empty()) {
				postGroup = gMap.at(postReaders.front());
				for (std::size_t i = 1; i < postReaders.size(); i++)
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

			for (std::size_t i = 0; i < rst.size(); i++) {
				auto& gi = rst[i];
				for (std::size_t j = i + 1; j < rst.size(); j++) {
					const auto& gj = rst[j];
					if (!NoneGroup::Parallelable(gi, gj))
						continue;

					bool haveOrder = false;
					for (auto* ifunc : gi.sysFuncs) {
						for (auto* jfunc : gj.sysFuncs) {
							if (graph.HavePath(ifunc, jfunc)
								|| graph.HavePath(jfunc, ifunc)) {
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
	sysFuncOrder.emplace(SystemFunc::GetValue(x), SystemFunc::GetValue(y));
	return *this;
}

Schedule& Schedule::InsertNone(string_view sys, TypeID type) {
	std::size_t hashcode = SystemFunc::GetValue(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseNones.erase(type);
	change.insertNones.insert(type);
	return *this;
}

Schedule& Schedule::EraseNone(string_view sys, TypeID type) {
	std::size_t hashcode = SystemFunc::GetValue(sys);
	auto& change = sysFilterChange[hashcode];
	change.eraseNones.insert(type);
	change.insertNones.erase(type);
	return *this;
}

void Schedule::Clear() {
	for (const auto& [hash, sysFunc] : sysFuncs) {
		sysFunc->~SystemFunc();
		sysFuncAllocator.deallocate(sysFunc, 1);
	}
	sysFuncs.clear();
	sysFuncOrder.clear();
	sysFilterChange.clear();
	sysLockFilter.clear();
}

unordered_map<TypeID, Schedule::CmptSysFuncs> Schedule::GenCmptSysFuncsMap() const {
	unordered_map<TypeID, CmptSysFuncs> rst;
	for (const auto& [hashcode, sysFunc] : sysFuncs) {
		for (const auto& type : sysFunc->entityQuery.locator.AccessTypeIDs()) {
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
	unordered_map<SystemFunc*, details::NoneGroup> groupMap;
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
		details::Compiler::SetPrePostEdge(graph, cmptSysFuncs, groupMap);

	// [gen graph] - edge - none group
	for (const auto& [type, cmptSysFuncs] : cmptSysFuncsMap) {
		if (cmptSysFuncs.writeSysFuncs.empty())
			continue;

		auto sortedGroup = details::Compiler::GenSortNoneGroup(graph, cmptSysFuncs, groupMap);
		
		for (std::size_t i = 0; i < sortedGroup.size() - 1; i++) {
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
