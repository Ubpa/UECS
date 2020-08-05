#include <UECS/World.h>

#include <UECS/IListener.h>

using namespace Ubpa::UECS;
using namespace Ubpa;
using namespace std;

World::World() : systemMngr{ this } {}

void World::Update() {
	schedule.Clear();
	for (auto job : jobs)
		jobPool.Recycle(job);
	jobs.clear();
	jobGraph.clear();

	for (auto& [id, system] : systemMngr.systems)
		system->OnUpdate(schedule);

	auto graph = schedule.GenSysFuncGraph();

	unordered_map<SystemFunc*, JobHandle> table;

	for (const auto& [func, adjVs] : graph.GetAdjList()) {
		auto job = jobPool.Request(func->Name());
		jobs.push_back(job);
		switch (func->GetMode())
		{
		case Ubpa::UECS::SystemFunc::Mode::Entity:
			entityMngr.GenEntityJob(this, job, func);
			break;
		case Ubpa::UECS::SystemFunc::Mode::Chunk:
			entityMngr.GenChunkJob(this, job, func);
			break;
		case Ubpa::UECS::SystemFunc::Mode::Job:
			job->emplace([this, func = func]() {
				(*func)(this);
			});
			break;
		default:
			break;
		}
		table[func] = jobGraph.composed_of(*job);
	}

	for (const auto& [v, adjVs] : graph.GetAdjList()) {
		auto vJob = table[v];
		for (auto adjV : adjVs)
			vJob.precede(table[adjV]);
	}

	executor.run(jobGraph).wait();

	RunCommands();
}

string World::DumpUpdateJobGraph() const {
	return jobGraph.dump();
}

// after running Update
UGraphviz::Graph World::GenUpdateFrameGraph() const {
	UGraphviz::Graph graph("Update Frame Graph", true);

	graph
		.RegisterGraphNodeAttr("style", "filled")
		.RegisterGraphNodeAttr("fontcolor", "white")
		.RegisterGraphNodeAttr("fontname", "consolas");

	auto& registry = graph.GetRegistry();

	auto& subgraph_cmpt = graph.GenSubgraph("Component Nodes");
	auto& subgraph_sys = graph.GenSubgraph("System Function Nodes");

	auto& subgraph_lastframe = graph.GenSubgraph("LastFrame Edges");
	auto& subgraph_write = graph.GenSubgraph("Write Edges");
	auto& subgraph_latest = graph.GenSubgraph("Latest Edges");

	auto& subgraph_order = graph.GenSubgraph("Order Edges");

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

	subgraph_order.RegisterGraphEdgeAttr("color", "#00A2E8");

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
	unordered_map<size_t, size_t> sysFuncHashcode2idx;

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
		auto cmptIdx = registry.RegisterNode(queryCmptName(cmptType));
		cmptType2idx[cmptType] = cmptIdx;
		subgraph_cmpt.AddNode(cmptIdx);
	}

	for (const auto& [hash, sysFunc] : schedule.sysFuncs) {
		auto sysIdx = registry.RegisterNode(sysFunc->Name());
		sysFuncHashcode2idx.emplace(hash, sysIdx);

		subgraph_sys.AddNode(sysIdx);

		const auto& locator = sysFunc->query.locator;
		for (const auto& cmptType : locator.LastFrameCmptTypes()) {
			auto edgeIdx = registry.RegisterEdge(cmptType2idx[cmptType], sysIdx);
			subgraph_lastframe.AddEdge(edgeIdx);
		}
		for (const auto& cmptType : locator.WriteCmptTypes()) {
			auto edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
			subgraph_write.AddEdge(edgeIdx);
		}
		for (const auto& cmptType : locator.LatestCmptTypes()) {
			auto edgeIdx = registry.RegisterEdge(cmptType2idx[cmptType], sysIdx);
			subgraph_latest.AddEdge(edgeIdx);
		}

		const auto& filter = sysFunc->query.filter;
		if (sysFunc->GetMode() == SystemFunc::Mode::Chunk) {
			// filter's <All> and <Any> components are treat as r/w
			for (const auto& cmptType : filter.AllCmptTypes()) {
				auto cmptIdx = cmptType2idx[cmptType];
				size_t edgeIdx;
				switch (cmptType.GetAccessMode())
				{
				case Ubpa::UECS::AccessMode::LAST_FRAME:
					edgeIdx = registry.RegisterEdge(cmptType2idx[cmptType], sysIdx);
					subgraph_lastframe.AddEdge(edgeIdx);
					break;
				case Ubpa::UECS::AccessMode::WRITE:
					edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
					subgraph_write.AddEdge(edgeIdx);
					break;
				case Ubpa::UECS::AccessMode::LATEST:
					edgeIdx = registry.RegisterEdge(cmptType2idx[cmptType], sysIdx);
					subgraph_latest.AddEdge(edgeIdx);
					break;
				default:
					assert(false);
					break;
				}
			}
			for (const auto& cmptType : filter.AnyCmptTypes()) {
				auto cmptIdx = cmptType2idx[cmptType];
				size_t edgeIdx;
				switch (cmptType.GetAccessMode())
				{
				case Ubpa::UECS::AccessMode::LAST_FRAME:
					edgeIdx = registry.RegisterEdge(cmptType2idx[cmptType], sysIdx);
					subgraph_lastframe.AddEdge(edgeIdx);
					break;
				case Ubpa::UECS::AccessMode::WRITE:
					edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
					subgraph_write.AddEdge(edgeIdx);
					break;
				case Ubpa::UECS::AccessMode::LATEST:
					edgeIdx = registry.RegisterEdge(cmptType2idx[cmptType], sysIdx);
					subgraph_latest.AddEdge(edgeIdx);
					break;
				default:
					assert(false);
					break;
				}
			}
		}
		else {
			for (const auto& cmptType : filter.AllCmptTypes()) {
				auto cmptIdx = cmptType2idx[cmptType];
				if (registry.IsRegisteredEdge(sysIdx, cmptIdx))
					continue;
				auto edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
				subgraph_all.AddEdge(edgeIdx);
			}
			for (const auto& cmptType : filter.AnyCmptTypes()) {
				auto cmptIdx = cmptType2idx[cmptType];
				if (registry.IsRegisteredEdge(sysIdx, cmptIdx))
					continue;
				auto edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
				subgraph_any.AddEdge(edgeIdx);
			}
		}
		for (const auto& cmptType : filter.NoneCmptTypes()) {
			auto cmptIdx = cmptType2idx[cmptType];
			if (registry.IsRegisteredEdge(sysIdx, cmptIdx))
				continue;
			auto edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx[cmptType]);
			subgraph_none.AddEdge(edgeIdx);
		}
	}

	for (const auto& [from, to] : schedule.sysFuncOrder) {
		auto fromIdx = sysFuncHashcode2idx.find(from)->second;
		auto toIdx = sysFuncHashcode2idx.find(to)->second;
		auto edgeIdx = registry.RegisterEdge(fromIdx, toIdx);
		subgraph_order.AddEdge(edgeIdx);
	}

	return graph;
}

void World::Accept(IListener* listener) const {
	listener->EnterWorld(this);
	systemMngr.Accept(listener);
	entityMngr.Accept(listener);
	listener->ExistWorld(this);
}

void World::AddCommand(std::function<void(World*)> command) {
	std::lock_guard<std::mutex> guard(commandBufferMutex);
	commandBuffer.push_back(std::move(command));
}

void World::RunCommands() {
	for (const auto& command : commandBuffer)
		command(this);
	commandBuffer.clear();
}
