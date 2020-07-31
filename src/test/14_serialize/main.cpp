#include <UECS/World.h>
#include <UECS/IListener.h>

#include <iostream>

using namespace Ubpa::UECS;
using namespace std;

struct Position { float val; };
struct Velocity { float val; };

class Dumper : public IListener {
	size_t indent{ 0 };
	bool firstSystem{ true };
	bool firstEntity{ true };
	bool firstCmpt{ true };

	void PrintIndent() {
		for (size_t i = 0; i < indent; i++)
			cout << "  ";
	}

	// new line
	virtual void EnterWorld(const World* w) override {
		cout << "{" << endl;
		indent++;
		PrintIndent();
		cout << "\"type\" : \"World\"," << endl;
	}

	virtual void ExistWorld(const World* w) override {
		indent--;
		cout << "}";
	}

	virtual void EnterSystemMngr(const SystemMngr* sm) override {
		PrintIndent();
		cout << "\"systemMngr\" : {" << endl;
		indent++;
		PrintIndent();
		cout << "\"type\" : \"SystemMngr\"," << endl;
		PrintIndent();
		cout << "\"systems\" : [";
		indent++;
		firstSystem = true;
	}

	virtual void ExistSystemMngr(const SystemMngr* sm) override {
		cout << endl;
		indent--;
		PrintIndent();
		cout << "]" << endl;
		indent--;
		PrintIndent();
		cout << "}," << endl;
	}

	virtual void EnterSystem(const System* s) override {
		if (!firstSystem)
			cout << ",";
		else
			firstSystem = false;
		cout << endl;
		PrintIndent();
		cout << "\"" << s->GetName() << "\"";
	}

	virtual void ExistSystem(const System* s) override {

	}

	virtual void EnterEntityMngr(const EntityMngr* em) override {
		PrintIndent();
		cout << "\"entityMngr\" : {" << endl;
		indent++;
		PrintIndent();
		cout << "\"type\" : \"EntityMngr\"," << endl;
		PrintIndent();
		cout << "\"entities\" : [";
		indent++;
		firstEntity = true;
	}

	virtual void ExistEntityMngr(const EntityMngr* em) override {
		cout << endl;
		indent--;
		PrintIndent();
		cout << "]" << endl;
		indent--;
		PrintIndent();
		cout << "}" << endl;
	}

	virtual void EnterEntity(const Entity* e) override {
		if (!firstEntity)
			cout << ",";
		else
			firstEntity = false;
		cout << endl;
		PrintIndent();
		cout << "{" << endl;
		indent++;
		PrintIndent();
		cout << "\"type\" : \"Entity\"," << endl;
		PrintIndent();
		cout << "\"components\" : [" << endl;
		indent++;
		firstCmpt = true;
	}

	virtual void ExistEntity(const Entity* e) override {
		cout << endl;
		indent--;
		PrintIndent();
		cout << "]" << endl;
		indent--;
		PrintIndent();
		cout << "}";
	}

	virtual void EnterCmptPtr(const CmptPtr* cmpt) override {
		if (!firstCmpt)
			cout << ",";
		else
			firstCmpt = false;
		cout << endl;
		PrintIndent();
		cout << "{" << endl;
		indent++;
		PrintIndent();
		cout << "\"type\" : \"" << RTDCmptTraits::Instance().Nameof(cmpt->Type()) << "\"";
		if (cmpt->Type().Is<Velocity>()) {
			auto v = cmpt->As<Velocity>();
			cout << "," << endl;
			PrintIndent();
			cout << "\"val\" : " << v->val;
		}
		else if (cmpt->Type().Is<Position>()) {
			auto p = cmpt->As<Position>();
			cout << "," << endl;
			PrintIndent();
			cout << "\"val\" : " << p->val;
		}
	}

	virtual void ExistCmptPtr(const CmptPtr* cmpt) override {
		cout << endl;
		indent--;
		PrintIndent();
		cout << "}";
	}
};

class MoverSystem : public System {
public:
	using System::System;

	virtual void OnUpdate(Schedule& schedule) override {
		schedule.Register(
			[](const Velocity* v, Position* p) {
				p->val += v->val;
			}, "Mover");
	}
};

int main() {
	World w;
	RTDCmptTraits::Instance().Register<Position, Velocity>();
	w.systemMngr.Register<MoverSystem>();
	w.entityMngr.Create<Position, Velocity>();
	w.entityMngr.Create<Position>();
	w.entityMngr.Create<Velocity>();
	w.entityMngr.Create<>();
	w.Update();
	Dumper dumper;
	w.Accept(&dumper);
}
