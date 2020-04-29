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
	struct MyUpdateSystem {
		static constexpr std::string_view name = "Writer7::MyUpdateSystem";
		int num = 7;
		void operator()(Data* data) const {
			cout << num << endl;
		}
	};

	static void OnSchedule(SystemSchedule<SysType::OnUpdate>& schedule) {
		string sname = string(MyUpdateSystem::name);
		schedule.Register(sname, MyUpdateSystem{})
			.After<Writer1>(sname)
			.Before<Writer5>(MyUpdateSystem::name);
	}
};
struct AfterRead1 { void OnUpdate(const Data* data) const { cout << "AfterRead1" << endl; } };
struct AfterRead2 { void OnUpdate(const Data* data) const { cout << "AfterRead2" << endl; } };

int main() {
	GetCmptSys<Writer2, SysType::OnUpdate>();

	CmptRegistrar::Instance().Register<Data, Writer1, Writer2, Writer3, Writer4, Writer5, Writer6,
		PreRead1, PreRead2, AfterRead1, AfterRead2>();
	World w;
	w.CreateEntity<Data, Writer1, Writer2, Writer3, Writer4, Writer5, Writer6,
		PreRead1, PreRead2, AfterRead1, AfterRead2>();
	w.Register<Writer7>();
	w.Start();
	w.Update();
	w.Stop();
	cout << w.DumpUpdateTaskflow() << endl;
	return 0;
}
