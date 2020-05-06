#include <UECS/World.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct A {
	size_t ID;
	void OnUpdate() const {
		cout << "A::OnUpdate() const: " << ID << endl;
	}
};

struct B {
	B(EntityPtr e, size_t n) : entity{ e }, num{ n }{}
	EntityPtr entity;
	size_t num{ 0 };

	void OnUpdate() {
		num--;
		entity->AddCommand([this]() {
			auto [e, a] = entity->World()->CreateEntity<A>();
			a->ID = num;
		});
		if (num == 0) {
			entity->AddCommand([this]() {
				this->entity->Release();
				cout << "Entity::Release" << endl;
			});
		}
	}
};

int main() {
	CmptRegistrar::Instance().Register<A, B>();

	World w;
	auto [e] = w.CreateEntity<>();
	e->AssignAttach<B>(e, static_cast<size_t>(5));
	for (size_t i = 0; i < 10; i++) {
		w.Update();
	}
	return 0;
}
