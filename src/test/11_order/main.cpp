#include <UECS/World.h>
#include <UECS/CmptTag.h>

#include <iostream>

using namespace std;
using namespace Ubpa;

struct Data {};

struct PreRead1 { void OnUpdate(CmptTag::LastFrame<Data> data) const { cout << "PreRead1" << endl; } };
struct PreRead2 { void OnUpdate(CmptTag::LastFrame<Data> data) const { cout << "PreRead2" << endl; } };
struct Writer1 { void OnUpdate(Data* data) const { cout << "Writer1" << endl; } };
struct Writer3 { void OnUpdate(Data* data) const { cout << "Writer3" << endl; } };
struct Writer4 { void OnUpdate(CmptTag::Before<Writer1>, Data * data) const { cout << "Writer4" << endl; } };
struct Writer6 { void OnUpdate(CmptTag::Before<Writer3>, Data * data) const { cout << "Writer6" << endl; } };
struct Writer2 { void OnUpdate(CmptTag::Before<Writer4, Writer6>, Data* data) const { cout << "Writer2" << endl; } };
struct Writer5 { void OnUpdate(CmptTag::After<Writer1>, Data * data) const { cout << "Writer5" << endl; } };
struct Writer7 {
	void MyUpdate(Data* data) const {
		cout << "7" << endl;
	}

	static void OnSchedule(SystemSchedule<SysType::OnUpdate>& schedule) {
		SystemSchedule<SysType::OnUpdate>::Config config;
		config.After<Writer1>()
			.Before<Writer5>();
		schedule.Regist(&MyUpdate, config);
	}
};
struct AfterRead1 { void OnUpdate(const Data* data) const { cout << "AfterRead1" << endl; } };
struct AfterRead2 { void OnUpdate(const Data* data) const { cout << "AfterRead2" << endl; } };

int main() {
	CmptRegister::Instance().Regist<Data, Writer1, Writer2, Writer3, Writer4, Writer5, Writer6, Writer7,
		PreRead1, PreRead2, AfterRead1, AfterRead2>();
	World w;
	w.CreateEntity<Data, Writer1, Writer2, Writer3, Writer4, Writer5, Writer6, Writer7,
		PreRead1, PreRead2, AfterRead1, AfterRead2>();
	w.Start();
	w.Update();
	w.Stop();
	cout << w.DumpUpdateTaskflow() << endl;
	return 0;
}
