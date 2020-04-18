#include <UECS/SystemSchedule.h>

using namespace Ubpa;
using namespace std;

System* SystemSchedule::RequestSystem(std::string_view name) {
	System* sys = syspool.Request(string(name));
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
		syspool.Recycle(sys);
	requestedSysVec.clear();
	id2rw.clear();
}

bool SystemSchedule::GenTaskflow(tf::Taskflow& taskflow) const {
	if (!IsDAG())
		return false;

	unordered_map<System*, tf::Task> sys2task;

	for (auto& [ID, rw] : id2rw) {
		tf::Task backWriterTask;
		tf::Task preTask;
		for (size_t i = 0; i < rw.writers.size(); i++) {
			size_t idx = rw.writers.size() - i - 1;
			size_t preIdx = idx + 1;
			tf::Task writerTask;
			auto writerTarget = sys2task.find(rw.writers[idx]);
			if (writerTarget == sys2task.end()) {
				writerTask = taskflow.composed_of(*rw.writers[idx]);
				sys2task[rw.writers[idx]] = writerTask;
			}
			else
				writerTask = writerTarget->second;
			if (!preTask.empty())
				writerTask.precede(preTask);
			else
				backWriterTask = writerTask;
			preTask = writerTask;
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

			if (!backWriterTask.empty())
				readerTask.succeed(backWriterTask);
		}
	}

	return true;
}

bool SystemSchedule::IsDAG() const noexcept {
	std::map<System*, std::vector<System*>> graph;
	for (const auto& [ID, rw] : id2rw) {
		for (const auto& writer : rw.writers) {
			auto& children = graph[writer];
			for (auto reader : rw.readers) {
				if (reader != writer)
					children.push_back(reader);
			}
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
