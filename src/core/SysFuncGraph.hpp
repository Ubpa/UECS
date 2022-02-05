#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory_resource>
#include <span>

namespace Ubpa::UECS {
	class SystemFunc;
	class SysFuncGraph {
	public:
		using AdjList = std::pmr::unordered_map<SystemFunc*, std::pmr::unordered_set<SystemFunc*>>;
		using allocator_type = AdjList::allocator_type;
		allocator_type get_allocator() const noexcept { return adjList.get_allocator(); }

		SysFuncGraph(const allocator_type& alloc) : adjList{ alloc } {}

		void AddVertex(SystemFunc* x);
		void AddEdge(SystemFunc* x, SystemFunc* y);

		bool HaveVertex(SystemFunc* x) const;
		bool HaveVertices(std::span<SystemFunc* const> vertices) const;
		bool HaveEdge(SystemFunc* x, SystemFunc* y) const;
		bool HavePath(SystemFunc* x, SystemFunc* y) const;

		void SubGraph(SysFuncGraph& rst, std::span<SystemFunc* const> vertices) const;
		std::tuple<bool, std::pmr::vector<SystemFunc*>> Toposort() const;
		bool IsDAG() const;

		const AdjList& GetAdjList() const noexcept { return adjList; }

	private:
		std::pmr::unordered_map<SystemFunc*, std::pmr::unordered_set<SystemFunc*>> adjList;
	};
}
