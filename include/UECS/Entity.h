#pragma once

#include "detail/EntityMngr.h"

namespace Ubpa {
	class Entity final : private EntityData {
		friend class EntityPtr;
		friend class EntityCPtr;
	public:
		template<typename Cmpt>
		Cmpt* Get();
		template<typename Cmpt>
		const Cmpt* Get() const;

		Ubpa::World* World() const noexcept;

		// ptr, id
		std::vector<CmptPtr> Components() const;

		template<typename... Cmpts>
		std::tuple<Cmpts *...> Attach();

		template<typename Cmpt, typename... Args>
		Cmpt* AssignAttach(Args... args);

		template<typename Cmpt>
		inline Cmpt* GetOrAttach();

		template<typename... Cmpts>
		void Detach();

		void Release() noexcept;

		// Attach, Detach, Release, World::CreateEntity
		// Run after World::Update, one by one, then will be cleared
		// run in main thread
		void AddCommand(const std::function<void()>& command);
	};

	static_assert(sizeof(Entity) == sizeof(EntityData) && std::is_base_of_v<EntityData, Entity>);

	class EntityPtr {
	public:
		EntityPtr(Entity* entity) : version{ entity ? entity->version : static_cast<size_t>(-1) }, entity{ entity } {}
		
		bool IsValid() const noexcept { return entity && version == entity->version; }

		Entity* operator->() const noexcept {
			assert(IsValid());
			return entity;
		}
		bool operator<(const EntityPtr& p) const noexcept {
			return entity < p.entity;
		}

	private:
		size_t version;
		Entity* entity;
	};

	class EntityCPtr {
	public:
		EntityCPtr(const Entity* entity) : version{ entity ? entity->version : static_cast<size_t>(-1) }, entity{ entity } {}

		bool IsValid() const noexcept { return entity && version == entity->version; }

		const Entity* operator->() const noexcept {
			assert(IsValid());
			return entity;
		}

		bool operator<(const EntityCPtr& p) const noexcept {
			return entity < p.entity;
		}

	private:
		size_t version;
		const Entity* entity;
	};
}

#include "detail/Entity.inl"
