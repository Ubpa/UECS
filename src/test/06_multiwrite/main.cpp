#include <UECS/World.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct Health {
	float value{ 100.f };
	void OnUpdate() const {
		cout << "Health::OnUpdate() const: " << value << endl;
	}
};

struct Hurt {
	float value{ 1.f };

	void OnUpdate(Health* health) const{
		health->value -= value;
	}
};

struct Restore {
	float value{ 0.8f };
	void OnUpdate(Health* health) const {
		health->value += value;
	}
};

int main() {
	World w;
	w.CreateEntity<Health, Hurt, Restore>();
	for (size_t i = 0; i < 20; i++) {
		w.Update();
	}
	return 0;
}
