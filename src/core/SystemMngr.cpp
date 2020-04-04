#include <UECS/detail/SystemMngr.h>

using namespace Ubpa;
using namespace std;

void SystemMngr::GenTaskflow(Table& table) const {
	table.finalSys.clear();
	table.id2rw.clear();
	table.tasks.clear();

	for (const auto& [ID, schedule] : schedules)
		schedule(table);

	assert(IsDAG(table));
	
	unordered_map<System*, tf::Task> sys2task;

	for (auto& [ID, rw] : table.id2rw) {
		tf::Task writerTask;
		if (rw.writer) {
			auto writerTarget = sys2task.find(rw.writer);
			if (writerTarget == sys2task.end()) {
				writerTask = table.finalSys.composed_of(*rw.writer);
				sys2task[rw.writer] = writerTask;
			}
			else
				writerTask = writerTarget->second;
		}

		for (auto reader : rw.readers) {
			tf::Task readerTask;
			auto readerTarget = sys2task.find(reader);
			if (readerTarget == sys2task.end()) {
				readerTask = table.finalSys.composed_of(*reader);
				sys2task[reader] = readerTask;
			}
			else
				readerTask = readerTarget->second;

			if(!writerTask.empty())
				readerTask.succeed(writerTask);
		}
	}
}

bool SystemMngr::IsDAG(const Table& table) {
	std::map<System*, std::vector<System*>> graph;
	for (const auto& [ID, rw] : table.id2rw) {
		if (!rw.writer)
			continue;
		auto& children = graph[rw.writer];
		for (auto reader : rw.readers) {
			if (reader != rw.writer)
				children.push_back(reader);
		}
	}

	std::set<System*> visited;
	for (const auto& [parent, children] : graph) {
		if (visited.find(parent) != visited.end())
			continue;
		std::deque<System*> sysQueue;
		sysQueue.push_back(parent);
		std::set<System*> curVisited;
		while (!sysQueue.empty()) {
			auto curSys = sysQueue.front();
			sysQueue.pop_front();
			if (curVisited.find(curSys) != curVisited.end())
				return false;
			if (visited.find(curSys) != visited.end())
				continue;
			curVisited.insert(curSys);
			visited.insert(curSys);
			for (const auto& child : graph[curSys])
				sysQueue.push_back(child);
		}
	}
	return true;
}
