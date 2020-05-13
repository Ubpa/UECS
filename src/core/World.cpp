#include <UECS/World.h>

using namespace Ubpa;
using namespace std;

void World::Update() {
	schedule.Clear();
	for (auto job : jobs)
		jobPool.Recycle(job);
	jobs.clear();
	jobGraph.clear();

	for (const auto& [id, lifecycle] : systemMngr.lifecycleMap)
		lifecycle.OnUpdate(schedule);
	auto graph = schedule.GenSysFuncGraph();

	unordered_map<SystemFunc*, JobHandle> table;

	for (const auto& [func, adjVs] : graph.GetAdjList()) {
		auto job = jobPool.Request(func->Name());
		jobs.push_back(job);
		if (!func->IsJob())
			entityMngr.GenJob(job, func);
		else
			job->emplace([func = func]() { (*func)(Entity::Invalid(), static_cast<size_t>(-1), nullptr); });
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

void World::AddCommand(const function<void()>& command) {
	lock_guard<mutex> guard(commandBufferMutex);
	commandBuffer.push_back(command);
}

void World::RunCommands() {
	lock_guard<mutex> guard(commandBufferMutex);
	for (const auto& command : commandBuffer)
		command();
	commandBuffer.clear();
}

string World::DumpUpdateJobGraph() {
	return jobGraph.dump();
}
