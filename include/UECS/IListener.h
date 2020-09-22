#pragma once

namespace Ubpa::UECS {
	class World;
	class EntityMngr;
	class Entity;
	class CmptPtr;

	class IListener {
	public:
		virtual void EnterWorld(const World*) = 0;
		virtual void ExistWorld(const World*) = 0;

		virtual void EnterEntityMngr(const EntityMngr*) = 0;
		virtual void ExistEntityMngr(const EntityMngr*) = 0;

		virtual void EnterEntity(Entity) = 0;
		virtual void ExistEntity(Entity) = 0;

		virtual void EnterCmpt(CmptPtr) = 0;
		virtual void ExistCmpt(CmptPtr) = 0;
	};
}
