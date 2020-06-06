#include <UECS/World.h>

using namespace Ubpa;
using namespace std;

void World::Update() {
	schedule.Clear();
	for (auto job : jobs)
		jobPool.Recycle(job);
	jobs.clear();
	jobGraph.clear();

	for (const auto& [id, onUpdate] : systemMngr.onUpdateMap)
		onUpdate(schedule);
	auto graph = schedule.GenSysFuncGraph();

	unordered_map<SystemFunc*, JobHandle> table;

	for (const auto& [func, adjVs] : graph.GetAdjList()) {
		auto job = jobPool.Request(func->Name());
		jobs.push_back(job);
		if (!func->IsJob())
			entityMngr.GenJob(job, func);
		else
			job->emplace([func = func]() { (*func)(Entity::Invalid(), size_t_invalid, RTDCmptsView{ nullptr,nullptr }); });
		table[func] = jobGraph.composed_of(*job);
	}

	for (const auto& [v, adjVs] : graph.GetAdjList()) {
		auto vJob = table[v];
		for (auto adjV : adjVs)
			vJob.precede(table[adjV]);
	}

	executor.run(jobGraph).wait();

	entityMngr.RunCommands();
}

string World::DumpUpdateJobGraph() const {
	return jobGraph.dump();
}

// after running Update
Graphviz::Graph World::GenUpdateFrameGraph() const {
	Graphviz::Graph graph("Update Frame Graph", true);

	graph
		.RegisterGraphNodeAttr("style", "filled")
		.RegisterGraphNodeAttr("fontcolor", "white")
		.RegisterGraphNodeAttr("fontname", "consolas");

	auto& registrar = graph.GetRegistrar();

	auto& subgraph_cmpt = graph.GenSubgraph("Component Nodes");
	auto& subgraph_sys = graph.GenSubgraph("System Function Nodes");

	auto& subgraph_lastframe = graph.GenSubgraph("LastFrame Edges");
	auto& subgraph_write = graph.GenSubgraph("Write Edges");
	auto& subgraph_latest = graph.GenSubgraph("Latest Edges");

	auto& subgraph_all = graph.GenSubgraph("All Edges");
	auto& subgraph_any = graph.GenSubgraph("Any Edges");
	auto& subgraph_none = graph.GenSubgraph("None Edges");

	subgraph_cmpt
		.RegisterGraphNodeAttr("shape", "ellipse")
		.RegisterGraphNodeAttr("color", "#6597AD");

	subgraph_sys
		.RegisterGraphNodeAttr("shape", "box")
		.RegisterGraphNodeAttr("color", "#F79646");

	subgraph_lastframe.RegisterGraphEdgeAttr("color", "#60C5F1");
	subgraph_write.RegisterGraphEdgeAttr("color", "#F47378");
	subgraph_latest.RegisterGraphEdgeAttr("color", "#6BD089");

	subgraph_all
		.RegisterGraphEdgeAttr("style", "dashed")
		.RegisterGraphEdgeAttr("color", "#C785C8")
		.RegisterGraphEdgeAttr("arrowhead", "crow");

	subgraph_any
		.RegisterGraphEdgeAttr("style", "dashed")
		.RegisterGraphEdgeAttr("color", "#C785C8")
		.RegisterGraphEdgeAttr("arrowhead", "diamond");

	subgraph_none
		.RegisterGraphEdgeAttr("style", "dashed")
		.RegisterGraphEdgeAttr("color", "#C785C8")
		.RegisterGraphEdgeAttr("arrowhead", "odot");

	unordered_set<CmptType> cmptTypes;
	unordered_map<CmptType, size_t> cmptType2idx;

	auto queryCmptName = [](CmptType type) -> string {
		auto cmptName = RTDCmptTraits::Instance().Nameof(type);
		return cmptName.empty() ? std::to_string(type.HashCode()) : string{ cmptName };
	};

	for (const auto& [hash, sysFunc] : schedule.sysFuncs) {
		for (auto cmptType : sysFunc->query.locator.CmptTypes())
			cmptTypes.insert(cmptType);
		for (auto cmptType : sysFunc->query.filter.AllCmptTypes())
			cmptTypes.insert(cmptType);
		for (auto cmptType : sysFunc->query.filter.AnyCmptTypes())
			cmptTypes.insert(cmptType);
		for (auto cmptType : sysFunc->query.filter.NoneCmptTypes())
			cmptTypes.insert(cmptType);
	}

	for (auto cmptType : cmptTypes) {
		auto cmptIdx = registrar.RegisterNode(queryCmptName(cmptType));
		cmptType2idx[cmptType] = cmptIdx;
		subgraph_cmpt.AddNode(cmptIdx);
	}

	for (const auto& [hash, sysFuncs] : schedule.sysFuncs) {
		auto sysIdx = registrar.RegisterNode(sysFuncs->Name());
		subgraph_sys.AddNode(sysIdx);

		const auto& locator = sysFuncs->query.locator;
		for (const auto& cmptType : locator.LastFrameCmptTypes()) {
			auto edgeIdx = registrar.RegisterEdge(cmptType2idx[cmptType], sysIdx);
			subgraph_lastframe.AddEdge(edgeIdx);
		}
		for (const auto& cmptType : locator.WriteCmptTypes()) {
			auto edgeIdx = registrar.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
			subgraph_write.AddEdge(edgeIdx);
		}
		for (const auto& cmptType : locator.LatestCmptTypes()) {
			auto edgeIdx = registrar.RegisterEdge(cmptType2idx[cmptType], sysIdx);
			subgraph_latest.AddEdge(edgeIdx);
		}

		const auto& filter = sysFuncs->query.filter;
		for (const auto& cmptType : filter.AllCmptTypes()) {
			auto cmptIdx = cmptType2idx[cmptType];
			if (registrar.IsRegisteredEdge(sysIdx, cmptIdx))
				continue;
			auto edgeIdx = registrar.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
			subgraph_all.AddEdge(edgeIdx);
		}
		for (const auto& cmptType : filter.AnyCmptTypes()) {
			auto cmptIdx = cmptType2idx[cmptType];
			if (registrar.IsRegisteredEdge(sysIdx, cmptIdx))
				continue;
			auto edgeIdx = registrar.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
			subgraph_any.AddEdge(edgeIdx);
		}
		for (const auto& cmptType : filter.NoneCmptTypes()) {
			auto cmptIdx = cmptType2idx[cmptType];
			if (registrar.IsRegisteredEdge(sysIdx, cmptIdx))
				continue;
			auto edgeIdx = registrar.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
			subgraph_none.AddEdge(edgeIdx);
		}
	}

	return graph;
}
