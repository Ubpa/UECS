#include <UECS/detail/SysFuncGraph.h>

#include <unordered_set>
#include <queue>
#include <stack>

#include <cassert>

using namespace Ubpa;
using namespace std;

bool SysFuncGraph::HaveVertex(SystemFunc* x) const {
	return adjList.find(x) != adjList.end();
}

bool SysFuncGraph::HaveVertices(const std::vector<SystemFunc*>& vertices) const {
	for (auto v : vertices) {
		if (!HaveVertex(v))
			return false;
	}
	return true;
}

bool SysFuncGraph::HaveEdge(SystemFunc* x, SystemFunc* y) const {
	assert(HaveVertex(x) && HaveVertex(y));
	assert(x != y);
	const auto& adjVs = adjList.find(x)->second;
	auto target = find(adjVs.begin(), adjVs.end(), y);
	return target != adjVs.end();
}

bool SysFuncGraph::HavePath(SystemFunc* x, SystemFunc* y) const {
	assert(HaveVertex(x) && HaveVertex(y));
	assert(x != y);

	unordered_set<SystemFunc*> visited;
	queue<SystemFunc*> q;
	q.push(x);
	while (!q.empty()) {
		auto v = q.front();
		visited.insert(v);
		q.pop();
		for (auto adjV : adjList.find(v)->second) {
			if (visited.find(adjV) != visited.end())
				continue;
			if (y == adjV)
				return true;
			q.push(adjV);
		}
	}
	return false;
}

void SysFuncGraph::AddVertex(SystemFunc* x) {
	assert(!HaveVertex(x));
	adjList.emplace(x, unordered_set<SystemFunc*>{});
}

void SysFuncGraph::AddEdge(SystemFunc* x, SystemFunc* y) {
	assert(HaveVertex(x) && HaveVertex(y));
	adjList[x].insert(y);
}

SysFuncGraph SysFuncGraph::SubGraph(const std::vector<SystemFunc*>& vertices) const {
	assert(HaveVertices(vertices));
	SysFuncGraph subgraph;
	for (auto v : vertices)
		subgraph.AddVertex(v);

	for (auto x : vertices) {
		for (auto y : vertices) {
			if (y == x)
				continue;
			if (HavePath(x, y))
				subgraph.AddEdge(x, y);
		}
	}

	return subgraph;
}

tuple<bool, vector<SystemFunc*>> SysFuncGraph::Toposort() const {
	unordered_map<SystemFunc*, size_t> in_degree_map;

	for (const auto& [parent, children] : adjList)
		in_degree_map.emplace(parent, 0);

	for (const auto& [parent, children] : adjList) {
		for (const auto& child : children)
			in_degree_map[child] += 1;
	}

	stack<SystemFunc*> zero_in_degree_vertices;
	vector<SystemFunc*> sorted_vertices;

	for (const auto& [v, d] : in_degree_map) {
		if (d == 0)
			zero_in_degree_vertices.push(v);
	}

	while (!zero_in_degree_vertices.empty()) {
		auto v = zero_in_degree_vertices.top();
		zero_in_degree_vertices.pop();
		sorted_vertices.push_back(v);
		in_degree_map.erase(v);
		for (auto child : adjList.find(v)->second) {
			auto target = in_degree_map.find(child);
			if (target == in_degree_map.end())
				continue;
			target->second--;
			if (target->second == 0)
				zero_in_degree_vertices.push(child);
		}
	}

	if (!in_degree_map.empty())
		return { false, vector<SystemFunc*>{} };
	
	return { true, sorted_vertices };
}

bool SysFuncGraph::IsDAG() const {
	return get<bool>(Toposort());
}
