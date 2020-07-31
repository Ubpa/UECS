#pragma once

namespace Ubpa::UECS {
	class World;
	class SystemMngr;
	class System;
	class EntityMngr;
	class Entity;
	class CmptPtr;

	class IListener {
	public:
		virtual void EnterWorld(const World* w) = 0;
		virtual void ExistWorld(const World* w) = 0;

		virtual void EnterSystemMngr(const SystemMngr* sm) = 0;
		virtual void ExistSystemMngr(const SystemMngr* sm) = 0;

		virtual void EnterSystem(const System* s) = 0;
		virtual void ExistSystem(const System* s) = 0;

		virtual void EnterEntityMngr(const EntityMngr* em) = 0;
		virtual void ExistEntityMngr(const EntityMngr* em) = 0;

		virtual void EnterEntity(const Entity* e) = 0;
		virtual void ExistEntity(const Entity* e) = 0;

		virtual void EnterCmptPtr(const CmptPtr* cmpt) = 0;
		virtual void ExistCmptPtr(const CmptPtr* cmpt) = 0;
	};
}
