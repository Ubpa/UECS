#include <UECS/World.hpp>

#include <UECS/IListener.hpp>

#include "SysFuncGraph.hpp"

using namespace Ubpa::UECS;
using namespace Ubpa;
using namespace std;

World::World() :
	sync_rsrc{ std::make_unique<std::pmr::synchronized_pool_resource>() },
	unsync_rsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>() },
	sync_frame_rsrc{ std::make_unique<synchronized_monotonic_buffer_resource>(unsync_rsrc.get()) },
	unsync_frame_rsrc{ std::make_unique<std::pmr::monotonic_buffer_resource>(unsync_rsrc.get()) },
	systemMngr{ this },
	entityMngr{ this },
	schedule{ this }{}

World::World(std::pmr::memory_resource* upstream) :
	sync_rsrc{ std::make_unique<std::pmr::synchronized_pool_resource>(upstream) },
	unsync_rsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>(upstream) },
	sync_frame_rsrc{ std::make_unique<synchronized_monotonic_buffer_resource>(unsync_rsrc.get()) },
	unsync_frame_rsrc{ std::make_unique<std::pmr::monotonic_buffer_resource>(unsync_rsrc.get()) },
	systemMngr{ this },
	entityMngr{ this },
	schedule{ this }{}

World::World(const World& w) :
	sync_rsrc{ std::make_unique<std::pmr::synchronized_pool_resource>() },
	unsync_rsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>() },
	sync_frame_rsrc{ std::make_unique<synchronized_monotonic_buffer_resource>(unsync_rsrc.get()) },
	unsync_frame_rsrc{ std::make_unique<std::pmr::monotonic_buffer_resource>(unsync_rsrc.get()) },
	systemMngr{ w.systemMngr, this },
	entityMngr{ w.entityMngr, this },
	schedule{ w.schedule, this } { assert(!w.inRunningJobGraph); }

World::World(const World& w, std::pmr::memory_resource* upstream) :
	sync_rsrc{ std::make_unique<std::pmr::synchronized_pool_resource>(upstream) },
	unsync_rsrc{ std::make_unique<std::pmr::unsynchronized_pool_resource>(upstream) },
	sync_frame_rsrc{ std::make_unique<synchronized_monotonic_buffer_resource>(unsync_rsrc.get()) },
	unsync_frame_rsrc{ std::make_unique<std::pmr::monotonic_buffer_resource>(unsync_rsrc.get()) },
	systemMngr{ w.systemMngr, this },
	entityMngr{ w.entityMngr, this },
	schedule{ w.schedule, this } { assert(!w.inRunningJobGraph); }

World::World(World&& w) noexcept :
	sync_rsrc{ std::move(w.sync_rsrc) },
	unsync_rsrc{ std::move(w.unsync_rsrc) },
	sync_frame_rsrc{ std::move(w.sync_frame_rsrc) },
	unsync_frame_rsrc{ std::move(w.unsync_frame_rsrc) },
	systemMngr{ std::move(w.systemMngr), this },
	entityMngr{ std::move(w.entityMngr), this },
	schedule{ std::move(w.schedule), this } { assert(!w.inRunningJobGraph); }

World::~World() {
	schedule.Clear();
	systemMngr.Clear();
	entityMngr.Clear();
}

void World::Update() {
	// 1. clear
	schedule.Clear();

	sync_frame_rsrc->release();
	unsync_frame_rsrc->release();

	entityMngr.NewFrame();

	// 2. update schedule

	for (const auto& ID : systemMngr.GetActiveSystemIDs())
		systemMngr.Update(ID, schedule);

	// 3. run job graph for several layers
	inRunningJobGraph = true;

	auto* table = UnsyncNewFrameObject<std::pmr::unordered_map<SystemFunc*, JobHandle>>();

	for (const auto& [layer, layerinfo] : schedule.layerInfos) {
		assert(layer != SpecialLayer);
		for (const auto& [hashcode, func] : layerinfo.sysFuncs) {
			auto* job = UnsyncNewFrameObject<Job>(std::string{ func->Name() });
			jobs.push_back(job);
			if (!entityMngr.AutoGen(this, job, func, layer))
				schedule.Disable(func->Name());
			table->emplace(func, jobGraph.composed_of(*job));
		}

		const auto* graph = schedule.GenSysFuncGraph(layer); // no need to delete

		for (const auto& [v, adjVs] : graph->GetAdjList()) {
			auto vJob = table->at(v);
			for (auto* adjV : adjVs)
				vJob.precede(table->at(adjV));
		}
		table->clear();
		executor.run(jobGraph).wait();
		RunCommands(layer);

		for (auto* job : jobs)
			job->~Taskflow();
		jobs.clear();
		jobGraph.clear();
	}
	inRunningJobGraph = false;

	// 4. update version
	version++;
}

string World::DumpUpdateJobGraph() const {
	return jobGraph.dump();
}

CommandBuffer World::Run(SystemFunc* sys) {
	if (sys->IsParallel()) {
		assert(!inRunningJobGraph);
		Job job;
		entityMngr.AutoGen(this, &job, sys, SpecialLayer);
		executor.run(job).wait();
	}
	else
		entityMngr.AutoGen(this, nullptr, sys, SpecialLayer);

	auto target = lcommandBuffer.find(SpecialLayer);
	if (target == lcommandBuffer.end())
		return {};
	CommandBuffer rst = std::move(target->second);
	lcommandBuffer.erase(target);
	return rst;
}

// after running Update
UGraphviz::Graph World::GenUpdateFrameGraph(int layer) const {
	assert(layer != SpecialLayer);
	std::string title = "Update Frame Graph [layer: " + std::to_string(layer) + "]";
	UGraphviz::Graph graph(std::move(title), true);

	const auto& layerinfo = schedule.layerInfos.at(layer);

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

	for (const auto& [hash, sysFunc] : layerinfo.sysFuncs) {
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

	for (const auto& [hash, sysFunc] : layerinfo.sysFuncs) {
		auto sysIdx = registry.RegisterNode(std::string{ sysFunc->Name() });
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

	for (const auto& [from, to] : layerinfo.sysFuncOrder) {
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

synchronized_monotonic_buffer_resource* World::GetSyncFrameResource() { return sync_frame_rsrc.get(); }
std::pmr::monotonic_buffer_resource* World::GetUnsyncFrameResource() { return unsync_frame_rsrc.get(); }
std::pmr::synchronized_pool_resource* World::GetSyncResource() { return sync_rsrc.get(); }
std::pmr::unsynchronized_pool_resource* World::GetUnsyncResource() { return unsync_rsrc.get(); }

std::uint64_t World::Version() const noexcept { return version; }

void World::AddCommand(std::function<void()> command, int layer) {
	std::lock_guard<std::mutex> guard(commandBufferMutex);
	lcommandBuffer[layer].AddCommand(command);
}

void World::AddCommandBuffer(CommandBuffer cb, int layer) {
	std::lock_guard<std::mutex> guard(commandBufferMutex);
	lcommandBuffer[layer].AddCommandBuffer(std::move(cb));
}

void World::RunCommands(int layer) {
	assert(layer != SpecialLayer);
	auto target = lcommandBuffer.find(layer);
	if (target == lcommandBuffer.end())
		return;
	auto& cb = target->second;
	for (const auto& cmd : cb.GetCommands())
		cmd();
	cb.Clear();
	lcommandBuffer.erase(target);
}
