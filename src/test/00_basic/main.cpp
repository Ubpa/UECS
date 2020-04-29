#include <UECS/World.h>
#include <iostream>

struct velocity { float value{ 0.f }; };
struct position { float value{ 0.f }; };

int main() {
	Ubpa::CmptRegistrar::Instance().Register<velocity, position>();

	Ubpa::World w;
	
	for (size_t i = 0; i < 10; i++) {
		auto [entity, v, p] = w.CreateEntity<velocity, position>();
		v->value = static_cast<float>(i);
	}

	float deltaT = 0.033f;

	w.Each([deltaT](velocity* v, position* p) {
		p->value += v->value * deltaT;
	});

	w.Each([](position* p) {
		std::cout << p->value << std::endl;
	});

	w.Each([](position* p)->bool {
		std::cout << "stop each" << std::endl;
		return false;
	});

	return 0;
}
