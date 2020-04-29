#include <UECS/Schedule.h>

using namespace Ubpa;
using namespace std;

Job* Schedule::RequestJob(const string& name) {
	Job* job = jobPool.Request(name);
	jobs[name] = job;
	return job;
}

void Schedule::Clear() {
	for (auto [name, sys] : jobs)
		jobPool.Recycle(sys);
	jobs.clear();
	id2rw.clear();
	sysOrderMap.clear();
}

bool Schedule::GenJobGraph(Job& job) const {
	if (!IsDAG())
		return false;

	unordered_map<Job*, tf::Task> job2task;

	for (auto& [ID, rw] : id2rw) {
		// toposort
		auto writers = rw.writers; // copy
		vector<Job*> sortedSystems; // last to first
		while (!writers.empty()) {
			auto iter_begin = writers.begin();
			deque<Job*> childrenQueue; // FIFO, push_back, pop_front
			childrenQueue.push_back(*iter_begin);

			vector<Job*> waitedSystems;

			while (!childrenQueue.empty()) {
				auto writer = childrenQueue.front();
				childrenQueue.pop_front();

				if (writers.find(writer) == writers.end())
					continue;

				waitedSystems.push_back(writer);
				writers.erase(writer); // directly change writers

				auto target_children = sysOrderMap.find(writer->name());
				if (target_children == sysOrderMap.end())
					continue;

				for (const auto& sys_name : target_children->second)
					childrenQueue.push_back(jobs.find(sys_name)->second);
			}

			for (auto iter = waitedSystems.rbegin(); iter != waitedSystems.rend(); ++iter)
				sortedSystems.push_back(*iter);
		}

		vector<tf::Task> writerTasks;
		for (size_t i = 0; i < sortedSystems.size(); i++) {
			tf::Task task;

			auto target = job2task.find(sortedSystems[i]);
			if (target == job2task.end()) {
				task = job.composed_of(*sortedSystems[i]);
				job2task[sortedSystems[i]] = task;
			}
			else
				task = target->second;

			if (!writerTasks.empty())
				task.precede(writerTasks.back());

			writerTasks.push_back(task);
		}

		for (auto pre_reader : rw.pre_readers) {
			tf::Task task;
			auto target = job2task.find(pre_reader);
			if (target == job2task.end()) {
				task = job.composed_of(*pre_reader);
				job2task[pre_reader] = task;
			}
			else
				task = target->second;

			if (!writerTasks.empty())
				task.precede(writerTasks.back());
		}

		for (auto post_reader : rw.post_readers) {
			tf::Task task;
			auto target = job2task.find(post_reader);
			if (target == job2task.end()) {
				task = job.composed_of(*post_reader);
				job2task[post_reader] = task;
			}
			else
				task = target->second;

			if (!writerTasks.empty())
				task.succeed(writerTasks.front());
		}
	}

	return true;
}

bool Schedule::IsDAG() const noexcept {
	map<Job*, set<Job*>> graph;
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

	set<Job*> visited;
	for (const auto& [parent, children] : graph) {
		if (visited.find(parent) != visited.end())
			continue;
		stack<Job*> jobStack;
		jobStack.push(parent);
		while (!jobStack.empty()) {
			auto curSys = jobStack.top();
			jobStack.pop();
			visited.insert(curSys);
			auto target = graph.find(curSys);
			if (target != graph.end()) {
				for (const auto& child : target->second) {
					if (child == parent)
						return false;
					if (visited.find(curSys) == visited.end())
						jobStack.push(child);
				}
			}
		}
	}
	return true;
}
