#include <UECS/World.h>

#include <functional>

#include <iostream>
#include <set>

using namespace std;
using namespace Ubpa;

struct velocity {
	velocity() = default;
	~velocity() {
		cout << "velocity release " << this << endl;
	}
	velocity(float v) :v(v) {};
	float v{ 0.f };
};

struct position {
	~position() {
		cout << "position release " << this << endl;
	}
	float x{ 0.f };
	position() = default;
	position(position&& p) noexcept {
		this->x = p.x;
		cout << "aha" << endl;
	}
};

int main() {
	CmptRegister::Instance().Regist<velocity, position>();

	World w;
	set<Entity*> entities;

	auto [e] = w.CreateEntity<>();
	e->Attach<velocity>();
	e->Attach<position>();
	e->Detach<velocity>();
	e->Detach<position>();
	e->Attach<position, velocity>();
	e->Detach<position, velocity>();
	for (size_t i = 0; i < 10; i++)
		w.CreateEntity<>();

	for (size_t i = 0; i < 100; i++)
	{
		auto [e, p] = w.CreateEntity<position>();
		//e->Release();
		entities.insert(e);
	}

	while (!entities.empty()) {
		auto iter = entities.begin();
		(*iter)->Release();
		entities.erase(iter);
	}

	//for (size_t i = 0; i < 100000; i++)
	//{
	//	(*entities.begin())->Release();
	//	entities.erase(entities.begin());
	//}

	// [ invalid ]
	//size_t i = 0;
	//w.Each([&i]() {
	//	cout << "i: " << i << endl;
	//	i++;
	//	});

	// [ invalid ]
	// w.CreateEntity<velocity, velocity>();

	return 0;
}
