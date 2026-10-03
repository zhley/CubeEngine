#pragma once

#include "Cube/Reflection/Type.h"

class EditorNodeAccess;

namespace Cube {
    class Node;

	// Game runtime data is declared private or protected;
    // Data requiring persistent storage should be declared public for reflection serialization.
	class Component {
	public:
        friend class Node;
        friend class ::EditorNodeAccess;

		Component() = default;
		explicit Component(Node* node) : node(node) {}
		virtual ~Component() = default;

		virtual void start(){}
		virtual void update(float delta){}
		virtual TypeID getType() const = 0;

		Node* getNode() const { return node; }

	protected:
		Node* node;
	};
}
