#pragma once

#include <set>

namespace Ubpa::Cmpt {
	// auto delete children
	struct Node {
		Entity* entity{ nullptr };
		Node* parent{ nullptr };
		std::set<Node*> children;

		Node(Entity* entity = nullptr, Node* parent = nullptr)
			: entity(entity), parent(parent) { }

		~Node() {
			for (auto child : children)
				child->entity->Release();
			children.clear();
			if (parent) {
				parent->children.erase(this);
				parent = nullptr;
			}
			entity = nullptr;
		}

		void AddChild(Node* child) {
			if (child->parent != nullptr)
				child->parent->DelChild(child);

			child->parent = this;
			children.insert(child);
		}

		void DelChild(Node* child) {
			assert(child == this);
			if (child->parent != this)
				return;

			children.erase(child);
			child->parent = nullptr;
		}

		bool IsDescendantOf(Node* node) const {
			if (this == node)
				return true;

			if (parent == nullptr)
				return false;

			return parent->IsDescendantOf(node);
		}

		Node(const Node& tree) = delete;
		Node& operator=(const Node& tree) = delete;
	};
}
