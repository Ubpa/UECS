#include <UECS/World.h>
#include <UECS/CmptTag.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct Data {};

struct Writer1 { void OnUpdate(Data* data) const { cout << "1" << endl; } };
struct Writer3 { void OnUpdate(Data* data) const { cout << "3" << endl; } };
struct Writer4 { void OnUpdate(CmptTag::Before<Writer1>, Data * data) const { cout << "4" << endl; } };
struct Writer6 { void OnUpdate(CmptTag::Before<Writer3>, Data * data) const { cout << "6" << endl; } };
struct Writer2 { void OnUpdate(CmptTag::Before<Writer4, Writer6>, Data* data) const { cout << "2" << endl; } };
struct Writer5 { void OnUpdate(CmptTag::After<Writer1>, Data * data) const { cout << "5" << endl; } };

int main() {
	CmptRegister::Instance().Regist<Data, Writer1, Writer2, Writer3, Writer4, Writer5, Writer6>();
	World w;
	w.CreateEntity<Data, Writer1, Writer2, Writer3, Writer4, Writer5, Writer6>();
	w.Start();
	w.Update();
	w.Stop();
	cout << w.DumpUpdateTaskflow() << endl;
	return 0;
}
