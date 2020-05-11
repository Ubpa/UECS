#include <UECS/Schedule.h>

#include <UContainer/Algorithm.h>

using namespace Ubpa;
using namespace std;

Schedule& Schedule::Order(size_t x, size_t y) {
	sysFuncOrder.emplace(x, y);
	return *this;
}

Schedule& Schedule::Order(string_view x, string_view y) {
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

vector<Schedule::NoneGroup> Schedule::GenSortNoneGroup(SysFuncGraph writerGraph,
	const vector<SystemFunc*>& preReaders,
	const vector<SystemFunc*>& writers,
	const vector<SystemFunc*>& postReaders)
{
	NoneGroup preGroup;
	if (!preReaders.empty()) {
		preGroup.sysFuncs.insert(preReaders.begin(), preReaders.end());
		preGroup.allTypes = SetUnion(preReaders.front()->query.Filter().AllCmptTypes(), preReaders.front()->query.Filter().AnyCmptTypes());
		preGroup.allTypes = SetUnion(preGroup.allTypes, preReaders.front()->query.Locator().CmptTypes());
		preGroup.noneTypes = preReaders.front()->query.Filter().NoneCmptTypes();
		for (size_t i = 1; i < preReaders.size(); i++) {
			auto allTypes = SetUnion(preReaders[i]->query.Filter().AllCmptTypes(), preReaders[i]->query.Filter().AnyCmptTypes());
			allTypes = SetUnion(preGroup.allTypes, preReaders[i]->query.Locator().CmptTypes());
			preGroup.allTypes = SetIntersection(preGroup.allTypes, allTypes);
			preGroup.noneTypes = SetIntersection(preGroup.noneTypes, preReaders[i]->query.Filter().NoneCmptTypes());
		}

		writerGraph.AddVertex(preReaders.front());
		for (auto writer : writers) {
			auto allTypes = SetUnion(writer->query.Filter().AllCmptTypes(), writer->query.Filter().AnyCmptTypes());
			allTypes = SetUnion(preGroup.allTypes, writer->query.Locator().CmptTypes());
			if (SetIntersection(preGroup.allTypes, writer->query.Filter().NoneCmptTypes()).empty()
				&& SetIntersection(allTypes, preGroup.noneTypes).empty())
				continue;
			writerGraph.AddEdge(preReaders.front(), writer);
		}
	}
	NoneGroup postGroup;
	if (!postReaders.empty()) {
		postGroup.sysFuncs.insert(postReaders.begin(), postReaders.end());
		postGroup.allTypes = SetUnion(postReaders.front()->query.Filter().AllCmptTypes(), postReaders.front()->query.Filter().AnyCmptTypes());
		postGroup.allTypes = SetUnion(postGroup.allTypes, postReaders.front()->query.Locator().CmptTypes());
		postGroup.noneTypes = postReaders.front()->query.Filter().NoneCmptTypes();
		for (size_t i = 1; i < postReaders.size(); i++) {
			auto allTypes = SetUnion(postReaders[i]->query.Filter().AllCmptTypes(), postReaders[i]->query.Filter().AnyCmptTypes());
			allTypes = SetUnion(postGroup.allTypes, postReaders[i]->query.Locator().CmptTypes());
			postGroup.allTypes = SetIntersection(postGroup.allTypes, allTypes);
			postGroup.noneTypes = SetIntersection(postGroup.noneTypes, postReaders[i]->query.Filter().NoneCmptTypes());
		}

		writerGraph.AddVertex(postReaders.front());
		for (auto writer : writers) {
			auto allTypes = SetUnion(writer->query.Filter().AllCmptTypes(), writer->query.Filter().AnyCmptTypes());
			allTypes = SetUnion(postGroup.allTypes, writer->query.Locator().CmptTypes());
			if (SetIntersection(postGroup.allTypes, writer->query.Filter().NoneCmptTypes()).empty()
				&& SetIntersection(allTypes, postGroup.noneTypes).empty())
				continue;
			writerGraph.AddEdge(writer, postReaders.front());
		}
	}

	auto [success, sortedFuncs] = writerGraph.Toposort();
	assert(success);

	vector<NoneGroup> rst;

	for (auto func : sortedFuncs) {
		if (!preReaders.empty() && func == preReaders.front())
			rst.push_back(move(preGroup));
		else if (!postReaders.empty() && func == postReaders.front())
			rst.push_back(move(postGroup));
		else {
			NoneGroup group;
			group.allTypes = SetUnion(func->query.Filter().AllCmptTypes(), func->query.Filter().AnyCmptTypes());
			group.allTypes = SetUnion(group.allTypes, func->query.Locator().CmptTypes());
			group.noneTypes = func->query.Filter().NoneCmptTypes();
			group.sysFuncs.insert(func);
			rst.push_back(move(group));
		}
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
					if (writerGraph.HavePath(ifunc, jfunc)
						|| writerGraph.HavePath(jfunc, ifunc)) {
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

	SysFuncGraph graph;

	// SystemFuncs related with <Cmpt>
	struct CmptSysFuncs {
		vector<SystemFunc*> lastFrameSysFuncs;
		vector<SystemFunc*> writeSysFuncs;
		vector<SystemFunc*> latestSysFuncs;
	};
	unordered_map<CmptType, CmptSysFuncs> cmptSysFuncsMap;

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
		const auto& cmptSysFuns = cmptSysFuncsMap[type];
		auto sortedGroup = GenSortNoneGroup(subgraph, cmptSysFuns.lastFrameSysFuncs, cmptSysFuns.writeSysFuncs, cmptSysFuns.latestSysFuncs);
		
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
