#pragma once

#include <unordered_map>
#include <vector>

namespace Ubpa {
	class SystemFunc;
	class SysFuncGraph {
	public:
		using AdjList = std::unordered_map<SystemFunc*, std::vector<SystemFunc*>>;

		void AddVertex(SystemFunc* x);
		void AddEdge(SystemFunc* x, SystemFunc* y);

		bool HaveVertex(SystemFunc* x) const;
		bool HaveVertices(const std::vector<SystemFunc*>& vertices) const;
		bool HaveEdge(SystemFunc* x, SystemFunc* y) const;
		bool HavePath(SystemFunc* x, SystemFunc* y) const;

		SysFuncGraph SubGraph(const std::vector<SystemFunc*>& vertices) const;
		std::tuple<bool, std::vector<SystemFunc*>> Toposort() const;
		bool IsDAG() const;

		const AdjList& GetAdjList() const noexcept { return adjList; }

	private:
		std::unordered_map<SystemFunc*, std::vector<SystemFunc*>> adjList;
	};
}
