#include "SysFuncGraph.h"

#include <unordered_set>
#include <queue>
#include <stack>

#include <cassert>

using namespace Ubpa::UECS;
using namespace std;

bool SysFuncGraph::HaveVertex(SystemFunc* x) const {
	return adjList.find(x) != adjList.end();
}

bool SysFuncGraph::HaveVertices(std::span<SystemFunc* const> vertices) const {
	for (auto v : vertices) {
		if (!HaveVertex(v))
			return false;
	}
	return true;
}

bool SysFuncGraph::HaveEdge(SystemFunc* x, SystemFunc* y) const {
	assert(HaveVertex(x) && HaveVertex(y));
	assert(x != y);
	const auto& adjVs = adjList.at(x);
	for(auto* adjV : adjVs){
		if(adjV == y)
			return true;
	}
	return false;
}

bool SysFuncGraph::HavePath(SystemFunc* x, SystemFunc* y) const {
	assert(HaveVertex(x) && HaveVertex(y));
	//if (x == y)
	//	return false; // acyclic

	unordered_set<SystemFunc*> visited;
	queue<SystemFunc*> q;
	q.push(x);
	while (!q.empty()) {
		auto* v = q.front();
		visited.insert(v);
		q.pop();
		for (auto* adjV : adjList.at(v)) {
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
	if (HaveVertex(x))
		return;
	adjList.emplace(x, std::pmr::unordered_set<SystemFunc*>(std::pmr::polymorphic_allocator<SystemFunc*>{adjList.get_allocator().resource()}));
}

void SysFuncGraph::AddEdge(SystemFunc* x, SystemFunc* y) {
	assert(HaveVertex(x) && HaveVertex(y));
	if (x == y)
		return;
	adjList[x].insert(y);
}

SysFuncGraph SysFuncGraph::SubGraph(std::span<SystemFunc* const> vertices) const {
	assert(HaveVertices(vertices));
	SysFuncGraph subgraph{ adjList.get_allocator().resource() };
	for (auto* v : vertices)
		subgraph.AddVertex(v);

	for (auto* x : vertices) {
		for (auto* y : vertices) {
			if (y == x)
				continue;
			if (HavePath(x, y))
				subgraph.AddEdge(x, y);
		}
	}

	return subgraph;
}

tuple<bool, std::pmr::vector<SystemFunc*>> SysFuncGraph::Toposort() const {
	unordered_map<SystemFunc*, std::size_t> in_degree_map;

	for (const auto& [parent, children] : adjList)
		in_degree_map.emplace(parent, 0);

	for (const auto& [parent, children] : adjList) {
		for (const auto& child : children)
			in_degree_map[child] += 1;
	}
	std::pmr::polymorphic_allocator<SystemFunc*> alloc{ adjList.get_allocator().resource() };

	stack<SystemFunc*, small_vector<SystemFunc*>> zero_in_degree_vertices;
	std::pmr::vector<SystemFunc*> sorted_vertices(alloc);

	for (const auto& [v, d] : in_degree_map) {
		if (d == 0)
			zero_in_degree_vertices.push(v);
	}

	while (!zero_in_degree_vertices.empty()) {
		auto* v = zero_in_degree_vertices.top();
		zero_in_degree_vertices.pop();
		sorted_vertices.push_back(v);
		in_degree_map.erase(v);
		for (auto* child : adjList.at(v)) {
			auto target = in_degree_map.find(child);
			if (target == in_degree_map.end())
				continue;
			target->second--;
			if (target->second == 0)
				zero_in_degree_vertices.push(child);
		}
	}

	if (!in_degree_map.empty())
		return { false, std::pmr::vector<SystemFunc*>(alloc) };
	
	return { true, sorted_vertices };
}

bool SysFuncGraph::IsDAG() const {
	return get<bool>(Toposort());
}
