#pragma once

#include "detail/ArchetypeMngr.h"

namespace Ubpa {
	class Entity final : private EntityBase {
	public:
		template<typename Cmpt>
		Cmpt* Get();
		template<typename Cmpt>
		const Cmpt* Get() const;

		Ubpa::World* World() const noexcept;

		const std::vector<std::tuple<void*, size_t>> Components() const;

		template<typename... Cmpts>
		std::tuple<Cmpts *...> Attach();

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
