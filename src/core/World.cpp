#include <UECS/World.h>

using namespace Ubpa;
using namespace std;

World::World()
	: entityMngr { this },
	SystemMngr{&entityMngr}
{
}

void World::Start() {
	SystemMngr::Start();
	
	entityMngr.RunCommands();
}

void World::Update() {
	SystemMngr::Update();

	entityMngr.RunCommands();
}

void World::Stop() {
	SystemMngr::Stop();

	entityMngr.RunCommands();
}

void World::AddCommand(const std::function<void()>& command) {
	entityMngr.AddCommand(command);
}
