#include <UECS/World.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};
struct F {};
struct G {};
struct H {};

struct MySystem {
	static void OnSchedule(ScheduleRegistrar<SysType::OnUpdate>& registrar) {
		registrar.Register("MySystem",
			[](
				// query
				CmptTag::All<A, B>,
				CmptTag::Any<C, D>,
				CmptTag::None<E>,
				// locate
				CmptTag::LastFrame<F> f,
				G* g, // CmptTag::Write<G>
				const H* h // CmptTag::Newest<G>
			)
			{
				cout << "MySystem" << endl;
			});
	}
};

int main() {
	CmptRegistrar::Instance().Register<A,B,C,D,E,F,G,H>();

	World w;
	w.Register<MySystem>();

	w.CreateEntity<A,B,C,F,G,H>();

	w.Start();
	w.Update();
	w.Stop();

	cout << w.DumpUpdateTaskflow() << endl;

	return 0;
}
