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
	const World* w;

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
		this->w = w;
	}

	virtual void ExistWorld(const World* w) override {
		indent--;
		cout << "}";
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

	virtual void EnterEntity(Entity e) override {
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

	virtual void ExistEntity(Entity e) override {
		cout << endl;
		indent--;
		PrintIndent();
		cout << "]" << endl;
		indent--;
		PrintIndent();
		cout << "}";
	}

	virtual void EnterCmpt(CmptPtr cmpt) override {
		if (!firstCmpt)
			cout << ",";
		else
			firstCmpt = false;
		cout << endl;
		PrintIndent();
		cout << "{" << endl;
		indent++;
		PrintIndent();
		cout << "\"type\" : \"" << w->entityMngr.cmptTraits.Nameof(cmpt.Type()) << "\"";
		if (cmpt.Type().Is<Velocity>()) {
			auto v = cmpt.As<Velocity>();
			cout << "," << endl;
			PrintIndent();
			cout << "\"val\" : " << v->val;
		}
		else if (cmpt.Type().Is<Position>()) {
			auto p = cmpt.As<Position>();
			cout << "," << endl;
			PrintIndent();
			cout << "\"val\" : " << p->val;
		}
	}

	virtual void ExistCmpt(CmptPtr cmpt) override {
		cout << endl;
		indent--;
		PrintIndent();
		cout << "}";
	}
};

struct MoverSystem {
	static void OnUpdate(Schedule& schedule) {
		schedule.RegisterEntityJob(
			[](const Velocity* v, Position* p) {
				p->val += v->val;
			}, "Mover");
	}
};

int main() {
	World w;
	w.entityMngr.cmptTraits.Register<Position, Velocity>();
	w.systemMngr.RegisterAndActivate<MoverSystem>();
	w.entityMngr.Create<Position, Velocity>();
	w.entityMngr.Create<Position>();
	w.entityMngr.Create<Velocity>();
	w.entityMngr.Create<>();
	w.Update();
	Dumper dumper;
	w.Accept(&dumper);
}
