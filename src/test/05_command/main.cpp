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
	Entity* entity{ nullptr };
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
	CmptRegister::Instance().Regist<A, B>();

	World w;
	auto [e, b] = w.CreateEntity<B>();
	b->num = 5;
	b->entity = e;
	for (size_t i = 0; i < 10; i++) {
		w.Update();
	}
	return 0;
}
