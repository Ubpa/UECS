#pragma once

#include <UTemplate/Type.h>

#include <functional>
#include <unordered_map>
#include <array>
#include <string>
#include <vector>
#include <memory_resource>

namespace Ubpa::UECS {
	class World;
	class Schedule;

	class SystemTraits {
	public:
		// [life cycle]
		//     OnCreate
		//         |
		//         v
		//     OnActivate <---
		//         |         |
		//         v         |
		// --> OnUpdate      |
		// |       |         |
		// --------x         |
		//         |         |
		//         v         |
		//     OnDeactivate  |
		//         |         |
		//         x----------
		//         |
		//         v
		//     OnDestroy
		using OnCreate     = void(World*);
		using OnActivate   = void(World*);
		using OnUpdate     = void(Schedule&);
		using OnDeactivate = void(World*);
		using OnDestroy    = void(World*);

		SystemTraits() = default;
		SystemTraits(const SystemTraits&);
		SystemTraits(SystemTraits&&) noexcept = default;
		SystemTraits& operator=(const SystemTraits&);
		SystemTraits& operator=(SystemTraits&&) noexcept = default;

		// register system's name and get an ID
		// if it is already registered, return it's ID directly
		Name Register(std::string_view name);

		// ID must exist
		void RegisterOnCreate    (NameID, std::function<OnCreate>    );
		void RegisterOnActivate  (NameID, std::function<OnActivate>  );
		void RegisterOnUpdate    (NameID, std::function<OnUpdate>    );
		void RegisterOnDeactivate(NameID, std::function<OnDeactivate>);
		void RegisterOnDestroy   (NameID, std::function<OnDestroy>   );

		std::string_view Nameof(NameID ID) const noexcept;
		bool IsRegistered(NameID ID) const noexcept;
		const auto& GetNames() const noexcept { return names; }

		// [ Template ] functions
		///////////////////////////

		template<typename Sys>
		static constexpr Name Nameof() noexcept;

		// for each <System> in <Systems...>
		// 1. Register(StaticNameof<System>())
		// 2. RegisterOn{Create|Activate|Update|Deactivate|Destroy} if <System> has them
		template<typename... Systems>
		std::array<Name, sizeof...(Systems)> Register();

		template<typename System>
		bool IsRegistered() const;

	private:
		friend class SystemMngr;

		void Create    (NameID, World*) const;
		void Activate  (NameID, World*) const;
		void Update    (NameID, Schedule&) const;
		void Deactivate(NameID, World*) const;
		void Destroy   (NameID, World*) const;

		std::pmr::synchronized_pool_resource rsrc;
		std::unordered_map<NameID, std::string_view> names;

		std::unordered_map<NameID, std::function<OnCreate    >> createMap;
		std::unordered_map<NameID, std::function<OnActivate  >> activateMap;
		std::unordered_map<NameID, std::function<OnUpdate    >> updateMap;
		std::unordered_map<NameID, std::function<OnDeactivate>> deactivateMap;
		std::unordered_map<NameID, std::function<OnDestroy   >> destroyMap;
	};
}

#include "details/SystemTraits.inl"
