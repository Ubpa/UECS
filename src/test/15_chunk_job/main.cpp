#include <UECS/World.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace Ubpa;
using namespace std;

struct S { float value; };
struct A { float value; };
struct B { float value; };

constexpr float dt = 0.003f;

class SAB_System : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		ArchetypeFilter filter;
		filter.all = { CmptType::Of<Write<S>> };
		filter.any = { CmptType::Of<Latest<A>>,CmptType::Of<Latest<B>> };

		schedule.RegisterChunkJob([](ChunkView chunk) {
			auto arrayS = chunk.GetCmptArray<S>();
			auto arrayA = chunk.GetCmptArray<A>();
			auto arrayB = chunk.GetCmptArray<B>();
			bool containsA = arrayA != nullptr;
			bool containsB = arrayB != nullptr;
			bool containsAB = containsA && containsB;

			if (containsAB) {
				cout << "[AB]" << endl;
				for (size_t i = 0; i < chunk.EntityNum(); i++) {
					arrayS[i].value += arrayA[i].value * arrayB[i].value;
				}
			}
			else if (containsA) {
				cout << "[A]" << endl;
				for (size_t i = 0; i < chunk.EntityNum(); i++) {
					arrayS[i].value += arrayA[i].value;
				}
			}
			else { // containsB
				cout << "[B]" << endl;
				for (size_t i = 0; i < chunk.EntityNum(); i++) {
					arrayS[i].value += arrayB[i].value;
				}
			}
		}, "SAB_System", filter);
	}
};

int main() {
	World w;
	w.systemMngr.Register<SAB_System>();

	w.entityMngr.cmptTraits.Register<S, A, B>();

	w.entityMngr.Create<S>();
	w.entityMngr.Create<S, A>();
	w.entityMngr.Create<S, B>();
	w.entityMngr.Create<S, A, B>();

	w.Update();

	cout << w.GenUpdateFrameGraph().Dump() << endl;

	return 0;
}
