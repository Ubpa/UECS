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
		virtual void EnterWorld(const World*) = 0;
		virtual void ExistWorld(const World*) = 0;

		virtual void EnterSystemMngr(const SystemMngr*) = 0;
		virtual void ExistSystemMngr(const SystemMngr*) = 0;

		virtual void EnterSystem(const System*) = 0;
		virtual void ExistSystem(const System*) = 0;

		virtual void EnterEntityMngr(const EntityMngr*) = 0;
		virtual void ExistEntityMngr(const EntityMngr*) = 0;

		virtual void EnterEntity(const Entity*) = 0;
		virtual void ExistEntity(const Entity*) = 0;

		virtual void EnterCmptPtr(const CmptPtr*) = 0;
		virtual void ExistCmptPtr(const CmptPtr*) = 0;
	};
}
