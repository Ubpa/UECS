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
		vector<tf::Task> writerTasks;
		for (size_t i = 0; i < rw.writers.size(); i++) {
			tf::Task task;

			auto target = sys2task.find(rw.writers[i]);
			if (target == sys2task.end()) {
				task = taskflow.composed_of(*rw.writers[i]);
				sys2task[rw.writers[i]] = task;
			}
			else
				task = target->second;

			if (!writerTasks.empty())
				task.succeed(writerTasks.back());
			
			writerTasks.push_back(task);
		}

		for (auto pre_reader : rw.pre_readers) {
			tf::Task task;
			auto target = sys2task.find(pre_reader);
			if (target == sys2task.end()) {
				task = taskflow.composed_of(*pre_reader);
				sys2task[pre_reader] = task;
			}
			else
				task = target->second;

			if (!writerTasks.empty())
				task.precede(writerTasks.front());
		}

		for (auto post_reader : rw.post_readers) {
			tf::Task task;
			auto target = sys2task.find(post_reader);
			if (target == sys2task.end()) {
				task = taskflow.composed_of(*post_reader);
				sys2task[post_reader] = task;
			}
			else
				task = target->second;

			if (!writerTasks.empty())
				task.succeed(writerTasks.back());
		}
	}

	return true;
}

bool SystemSchedule::IsDAG() const noexcept {
	std::map<System*, std::set<System*>> graph;
	for (const auto& [ID, rw] : id2rw) {
		for (const auto& writer : rw.writers) {
			for (auto post_reader : rw.post_readers) {
				if (post_reader != writer)
					graph[writer].insert(post_reader);
			}
			for (auto pre_reader : rw.pre_readers) {
				if (pre_reader != writer)
					graph[pre_reader].insert(writer);
			}
		}
	}

	std::set<System*> visited;
	for (const auto& [parent, children] : graph) {
		if (visited.find(parent) != visited.end())
			continue;
		std::set<System*> sysSet;
		sysSet.insert(parent);
		std::set<System*> curVisited;
		while (!sysSet.empty()) {
			auto curTarget = sysSet.begin();
			auto curSys = *curTarget;
			sysSet.erase(curTarget);
			if (curVisited.find(curSys) != curVisited.end())
				return false;
			if (visited.find(curSys) != visited.end())
				continue;
			curVisited.insert(curSys);
			visited.insert(curSys);
			auto target = graph.find(curSys);
			if (target != graph.end()) {
				for (const auto& child : target->second)
					sysSet.insert(child);
			}
		}
	}
	return true;
}
