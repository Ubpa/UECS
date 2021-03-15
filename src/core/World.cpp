#include <UECS/World.h>

#include <UECS/IListener.h>

#include "SysFuncGraph.h"

using namespace Ubpa::UECS;
using namespace Ubpa;
using namespace std;

World::World(const World& w)
	:
	jobRsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>() },
	systemMngr{ w.systemMngr, this },
	entityMngr{ w.entityMngr }
{}

World::World(World&& w) noexcept
	:
	jobRsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>() },
	systemMngr{ std::move(w.systemMngr), this },
	entityMngr{ std::move(w.entityMngr) }
{}

World::~World() {
	systemMngr.Clear();
}

void World::Update() {
	inRunningJobGraph = true;

	schedule.Clear();
	for (auto* job : jobs) {
		job->~Taskflow();
		jobRsrc->deallocate(job, sizeof(Job), alignof(Job));
	}
	jobs.clear();
	jobGraph.clear();

	for (const auto& ID : systemMngr.GetActiveSystemIDs())
		systemMngr.Update(ID, schedule);

	for (auto& [layer, scheduleCommands] : schedule.commandBuffer) {
		auto& worldCommands = commandBuffer[layer];
		for (auto& command : scheduleCommands) {
			worldCommands.emplace_back([this, command = std::move(command)](){
				command(this);
			});
		}
	}
	schedule.commandBuffer.clear();

	const auto graph = schedule.GenSysFuncGraph();

	unordered_map<SystemFunc*, JobHandle> table;

	for (const auto& [func, adjVs] : graph.GetAdjList()) {
		auto* job_buffer = jobRsrc->allocate(sizeof(Job), alignof(Job));
		auto* job = new(job_buffer)Job{ func->Name() };
		jobs.push_back(job);
		entityMngr.AutoGen(this, job, func);
		table.emplace(func, jobGraph.composed_of(*job));
	}

	for (const auto& [v, adjVs] : graph.GetAdjList()) {
		auto vJob = table.at(v);
		for (auto* adjV : adjVs)
			vJob.precede(table.at(adjV));
	}

	executor.run(jobGraph).wait();
	inRunningJobGraph = false;

	RunCommands();
}

string World::DumpUpdateJobGraph() const {
	return jobGraph.dump();
}

void World::Run(SystemFunc* sys) {
	if (sys->IsParallel()) {
		assert(!inRunningJobGraph);
		Job job;
		entityMngr.AutoGen(this, &job, sys);
		executor.run(job).wait();
	}
	else
		entityMngr.AutoGen(this, nullptr, sys);
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
	auto& subgraph_singleton = graph.GenSubgraph("Singleton Nodes");
	auto& subgraph_sys = graph.GenSubgraph("System Function Nodes");

	auto& subgraph_basic = graph.GenSubgraph("Basic Edges");
	auto& subgraph_lastframe = graph.GenSubgraph("LastFrame Edges");
	auto& subgraph_write = graph.GenSubgraph("Write Edges");
	auto& subgraph_latest = graph.GenSubgraph("Latest Edges");
	auto& subgraph_order = graph.GenSubgraph("Order Edges");

	auto& subgraph_basic_all = subgraph_basic.GenSubgraph("Basic All Edges");
	auto& subgraph_basic_any = subgraph_basic.GenSubgraph("Basic Any Edges");
	
	auto& subgraph_lastframe_any = subgraph_lastframe.GenSubgraph("LastFrame Any Edges");
	auto& subgraph_write_any = subgraph_write.GenSubgraph("Write Any Edges");
	auto& subgraph_latest_any = subgraph_latest.GenSubgraph("Latest Any Edges");
	
	auto& subgraph_none = subgraph_basic.GenSubgraph("None Edges");
	
	auto& subgraph_lastframe_random = subgraph_lastframe.GenSubgraph("LastFrame Random Edges");
	auto& subgraph_write_random = subgraph_write.GenSubgraph("Write Random Edges");
	auto& subgraph_latest_random = subgraph_latest.GenSubgraph("Latest Random Edges");

	subgraph_cmpt
		.RegisterGraphNodeAttr("shape", "ellipse")
		.RegisterGraphNodeAttr("color", "#6597AD");

	subgraph_singleton
		.RegisterGraphNodeAttr("shape", "ellipse")
		.RegisterGraphNodeAttr("color", "#BFB500");

	subgraph_sys
		.RegisterGraphNodeAttr("shape", "box")
		.RegisterGraphNodeAttr("color", "#F79646");

	subgraph_basic.RegisterGraphEdgeAttr("color", "#C785C8");
	subgraph_lastframe.RegisterGraphEdgeAttr("color", "#60C5F1");
	subgraph_write.RegisterGraphEdgeAttr("color", "#F47378");
	subgraph_latest.RegisterGraphEdgeAttr("color", "#6BD089");
	subgraph_order.RegisterGraphEdgeAttr("color", "#00A2E8");

	subgraph_basic_all.RegisterGraphEdgeAttr("style", "dashed");

	subgraph_basic_any
		.RegisterGraphEdgeAttr("style", "dashed")
		.RegisterGraphEdgeAttr("arrowhead", "diamond");
	subgraph_lastframe_any
		.RegisterGraphEdgeAttr("arrowhead", "diamond");
	subgraph_write_any
		.RegisterGraphEdgeAttr("arrowhead", "diamond");
	subgraph_latest_any
		.RegisterGraphEdgeAttr("arrowhead", "diamond");

	subgraph_none
		.RegisterGraphEdgeAttr("style", "dashed")
		.RegisterGraphEdgeAttr("arrowhead", "odot");
	
	subgraph_lastframe_random
		.RegisterGraphEdgeAttr("style", "dotted")
		.RegisterGraphEdgeAttr("dir", "both")
		.RegisterGraphEdgeAttr("arrowhead", "curve")
		.RegisterGraphEdgeAttr("arrowtail", "tee");
	subgraph_write_random
		.RegisterGraphEdgeAttr("style", "dotted")
		.RegisterGraphEdgeAttr("dir", "both")
		.RegisterGraphEdgeAttr("arrowhead", "curve")
		.RegisterGraphEdgeAttr("arrowtail", "tee");
	subgraph_latest_random
		.RegisterGraphEdgeAttr("style", "dotted")
		.RegisterGraphEdgeAttr("dir", "both")
		.RegisterGraphEdgeAttr("arrowhead", "curve")
		.RegisterGraphEdgeAttr("arrowtail", "tee");

	unordered_set<TypeID> cmptTypes;
	unordered_set<TypeID> singletonTypes;
	unordered_map<TypeID, std::size_t> cmptType2idx;
	unordered_map<std::size_t, std::size_t> sysFuncHashcode2idx;

	auto queryCmptName = [this](TypeID type) -> string {
		auto cmptName = entityMngr.cmptTraits.Nameof(type);
		return cmptName.empty() ? std::to_string(type.GetValue()) : string{ cmptName };
	};

	for (const auto& [hash, sysFunc] : schedule.sysFuncs) {
		for (auto cmptType : sysFunc->entityQuery.locator.AccessTypeIDs())
			cmptTypes.insert(cmptType);
		for (auto cmptType : sysFunc->entityQuery.filter.all)
			cmptTypes.insert(cmptType);
		for (auto cmptType : sysFunc->entityQuery.filter.any)
			cmptTypes.insert(cmptType);
		for (auto cmptType : sysFunc->entityQuery.filter.none)
			cmptTypes.insert(cmptType);
		for (auto singleton : sysFunc->singletonLocator.SingletonTypes())
			singletonTypes.insert(singleton);
	}

	for (const auto& cmptType : cmptTypes) {
		auto cmptIdx = registry.RegisterNode(queryCmptName(cmptType));
		cmptType2idx.emplace(cmptType, cmptIdx);
		subgraph_cmpt.AddNode(cmptIdx);
	}

	for (const auto& singletonType : singletonTypes) {
		auto cmptIdx = registry.RegisterNode(queryCmptName(singletonType));
		cmptType2idx.emplace(singletonType, cmptIdx);
		subgraph_singleton.AddNode(cmptIdx);
	}

	for (const auto& [hash, sysFunc] : schedule.sysFuncs) {
		auto sysIdx = registry.RegisterNode(sysFunc->Name());
		sysFuncHashcode2idx.emplace(hash, sysIdx);

		subgraph_sys.AddNode(sysIdx);

		for (const auto& cmptType : sysFunc->entityQuery.locator.AccessTypeIDs()) {
			std::size_t edgeIdx;
			switch (cmptType.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				edgeIdx = registry.RegisterEdge(cmptType2idx.at(cmptType), sysIdx);
				subgraph_lastframe.AddEdge(edgeIdx);
				break;
			case AccessMode::WRITE:
				edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx.at(cmptType));
				subgraph_write.AddEdge(edgeIdx);
				break;
			case AccessMode::LATEST:
				edgeIdx = registry.RegisterEdge(cmptType2idx.at(cmptType), sysIdx);
				subgraph_latest.AddEdge(edgeIdx);
				break;
			default:
				assert(false);
				break;
			}
		}

		const bool isChunk = sysFunc->GetMode() == SystemFunc::Mode::Chunk;

		for (const auto& cmptType : sysFunc->entityQuery.filter.all) {
			std::size_t edgeIdx;
			switch (cmptType.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				edgeIdx = registry.RegisterEdge(cmptType2idx.at(cmptType), sysIdx);
				(isChunk ? subgraph_lastframe : subgraph_basic).AddEdge(edgeIdx);
				break;
			case AccessMode::WRITE:
				edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx.at(cmptType));
				(isChunk ? subgraph_write : subgraph_basic).AddEdge(edgeIdx);
				break;
			case AccessMode::LATEST:
				edgeIdx = registry.RegisterEdge(cmptType2idx.at(cmptType), sysIdx);
				(isChunk ? subgraph_latest : subgraph_basic).AddEdge(edgeIdx);
				break;
			default:
				assert(false);
				break;
			}
		}

		for (const auto& cmptType : sysFunc->entityQuery.filter.any) {
			std::size_t edgeIdx;
			switch (cmptType.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				edgeIdx = registry.RegisterEdge(cmptType2idx.at(cmptType), sysIdx);
				(isChunk ? subgraph_lastframe_any : subgraph_basic_any).AddEdge(edgeIdx);
				break;
			case AccessMode::WRITE:
				edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx.at(cmptType));
				(isChunk ? subgraph_write_any : subgraph_basic_any).AddEdge(edgeIdx);
				break;
			case AccessMode::LATEST:
				edgeIdx = registry.RegisterEdge(cmptType2idx.at(cmptType), sysIdx);
				(isChunk ? subgraph_latest_any : subgraph_basic_any).AddEdge(edgeIdx);
				break;
			default:
				assert(false);
				break;
			}
		}

		for (const auto& cmptType : sysFunc->entityQuery.filter.none) {
			std::size_t edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx.at(cmptType));
			subgraph_none.AddEdge(edgeIdx);
		}

		for (const auto& cmptType : sysFunc->randomAccessor.types) {
			std::size_t edgeIdx;
			switch (cmptType.GetAccessMode())
			{
			case AccessMode::LAST_FRAME:
				edgeIdx = registry.RegisterEdge(cmptType2idx.at(cmptType), sysIdx);
				subgraph_lastframe_random.AddEdge(edgeIdx);
				break;
			case AccessMode::WRITE:
				edgeIdx = registry.RegisterEdge(sysIdx, cmptType2idx.at(cmptType));
				subgraph_write_random.AddEdge(edgeIdx);
				break;
			case AccessMode::LATEST:
				edgeIdx = registry.RegisterEdge(cmptType2idx.at(cmptType), sysIdx);
				subgraph_latest_random.AddEdge(edgeIdx);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	for (const auto& [from, to] : schedule.sysFuncOrder) {
		auto fromIdx = sysFuncHashcode2idx.at(from);
		auto toIdx = sysFuncHashcode2idx.at(to);
		auto edgeIdx = registry.RegisterEdge(fromIdx, toIdx);
		subgraph_order.AddEdge(edgeIdx);
	}

	return graph;
}

void World::Accept(IListener* listener) const {
	listener->EnterWorld(this);
	entityMngr.Accept(listener);
	listener->ExistWorld(this);
}

void World::AddCommand(std::function<void()> command, int layer) {
	assert(inRunningJobGraph);
	std::lock_guard<std::mutex> guard(commandBufferMutex);
	commandBuffer[layer].push_back(std::move(command));
}

void World::RunCommands() {
	for (const auto& [layer, commands] : commandBuffer) {
		for (const auto& command : commands)
			command();
	}
	commandBuffer.clear();
}
