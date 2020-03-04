#pragma once

#include "../core/Entity.h"

#include <set>

namespace Ubpa::Cmpt {
	// auto delete children
	class Node {
	public:
		Node(Ubpa::Entity* entity) : entity(entity) {}
		~Node();

		inline Ubpa::Entity* Entity() noexcept { return entity; }
		inline const Ubpa::Entity* Entity() const noexcept { return entity; }

		// don't call it in parallel
		void AddChild(Node* child);
		void DelChild(Node* child);

		bool IsDescendantOf(Node* e) const;

	private:
		Node(const Node& node) = delete;
		Node& operator=(const Node& node) = delete;

		Ubpa::Entity* entity;
		Node* parent{ nullptr };
		std::set<Node*> children;
	};
}
