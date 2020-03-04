#pragma once

#include "../core/Entity.h"

#include <set>

namespace Ubpa::Cmpt {
	// auto delete children
	struct Node {
		Entity* entity;
		Node* parent{ nullptr };
		std::set<Node*> children;

		Node(Entity* entity)
			: entity(entity) { }

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

		Node(const Node& node) = delete;
		Node& operator=(const Node& node) = delete;
	};
}
