#include <UECS/detail/SystemSchedule.h>

using namespace Ubpa;
using namespace std;

System* SystemSchedule::RequestSystem() {
	System* sys = syspool.request();
	requestedSysVec.push_back(sys);
	return sys;
}

SystemSchedule::SystemSchedule(ArchetypeMngr* mngr)
	: mngr{ mngr } {}

SystemSchedule::~SystemSchedule() {
	Clear();
}

void SystemSchedule::Clear() {
	for (auto sys : requestedSysVec)
		syspool.recycle(sys);
	requestedSysVec.clear();
	id2rw.clear();
}

bool SystemSchedule::GenTaskflow(tf::Taskflow& taskflow) const {
	if (!IsDAG())
		return false;

	unordered_map<System*, tf::Task> sys2task;

	for (auto& [ID, rw] : id2rw) {
		tf::Task writerTask;
		if (rw.writer) {
			auto writerTarget = sys2task.find(rw.writer);
			if (writerTarget == sys2task.end()) {
				writerTask = taskflow.composed_of(*rw.writer);
				sys2task[rw.writer] = writerTask;
			}
			else
				writerTask = writerTarget->second;
		}

		for (auto reader : rw.readers) {
			tf::Task readerTask;
			auto readerTarget = sys2task.find(reader);
			if (readerTarget == sys2task.end()) {
				readerTask = taskflow.composed_of(*reader);
				sys2task[reader] = readerTask;
			}
			else
				readerTask = readerTarget->second;

			if (!writerTask.empty())
				readerTask.succeed(writerTask);
		}
	}

	return true;
}

bool SystemSchedule::IsDAG() const noexcept {
	std::map<System*, std::vector<System*>> graph;
	for (const auto& [ID, rw] : id2rw) {
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
