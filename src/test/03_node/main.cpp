#include <UECS/core/World.h>
#include <UECS/cmpt/Node.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

int main() {
	World w;
	auto e0 = w.CreateEntity<Cmpt::Node>();
	auto e1 = w.CreateEntity<Cmpt::Node>();
	auto e2 = w.CreateEntity<Cmpt::Node>();
	auto e3 = w.CreateEntity<Cmpt::Node>();

	auto n0 = e0->Get<Cmpt::Node>();
	auto n1 = e1->Get<Cmpt::Node>();
	auto n2 = e2->Get<Cmpt::Node>();
	auto n3 = e3->Get<Cmpt::Node>();

	n0->entity = e0;
	n1->entity = e1;
	n2->entity = e2;
	n3->entity = e3;

	n0->AddChild(n1);
	n0->AddChild(n2);
	n2->AddChild(n3);

	return 0;
}
