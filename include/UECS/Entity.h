#pragma once

#include "EntityMngr.h"

namespace Ubpa {
	class Entity final : private EntityBase {
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

		bool IsAlive() const noexcept;

		void Release() noexcept;

		// Attach, Detach, Release, World::CreateEntity
		// Run after World::Update, one by one, then will be cleared
		// run in main thread
		void AddCommand(const std::function<void()>& command);
	};

	static_assert(sizeof(Entity) == sizeof(EntityBase) && std::is_base_of_v<EntityBase, Entity>);
}

#include "detail/Entity.inl"
